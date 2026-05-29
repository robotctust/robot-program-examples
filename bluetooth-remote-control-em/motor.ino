// =====================================================================
// 馬達穩速模組 (motor.ino)
//
// 此檔已封裝完成 — main.ino 只需透過下列 4 個 API 操作馬達，
// 不需要碰編碼器、PWM、PID 任何細節：
//
//   motorBegin()                    在 setup() 呼叫一次，初始化腳位/中斷
//   motorSetSpeed(rpmL, rpmR)       設定左右輪「目標轉速」(可正可負)
//   motorStop()                     立即停止並重置 PID 狀態
//   motorTick()                     在 loop() 每次呼叫，內部會自動依
//                                   SAMPLE_MS 節流，呼叫太頻繁不會出事
//
// 另提供高層動作 API（包裝上面的函數，speed 單位 RPM）：
//   Forward(speed) / Backward(speed) / SpinLeft(speed) / SpinRight(speed) / Stop()
//
// 設計重點：
//   - 用「設目標 / 跑 PID」分離的方式：呼叫 motorSetSpeed 只更新目標值，
//     真正的閉迴路控制由 motorTick 在固定週期執行。所以上層程式想設多快、
//     多頻繁都沒關係，PID 永遠是穩定的固定週期取樣（見 SAMPLE_MS）。
//   - 抗積分飽和：trial integral 通過後才確認，避免累積誤差爆衝。
//   - 換向時重置積分，避免反向起步殘留誤差。
// =====================================================================

// ===== 腳位（依硬體接線，一般不需修改）=====
static const int M_ENA   = 5,  M_IN1   = 6,  M_IN2 = 7;   // 右輪 (L293D OUTPUT 1/2)
static const int M_ENB   = 10, M_IN3   = 8,  M_IN4 = 9;   // 左輪 (L293D OUTPUT 3/4)
static const int M_ENCA_L = 2,  M_ENCB_L = 4;             // 左輪編碼器 (A 接中斷)
static const int M_ENCA_R = 3,  M_ENCB_R = 11;            // 右輪編碼器 (A 接中斷)

// =====================================================================
// ★ 馬達層可調參數 ★
// 以 encoder_motor_test1 整定後的數值為準，賽前一般不需修改
// =====================================================================
// 此馬達滿載 PWM(255) ≈ 50 RPM，故前饋斜率 ≈ 255/50 ≈ 5（PWM/RPM）。
static const double FF_MOTOR  = 5.0;    // 前饋（PWM/RPM），主要靠這個跟上目標
static const double KP_MOTOR  = 3.0;    // 比例修正
static const double KI_MOTOR  = 5.0;    // 積分修正（消除穩態誤差 / 克服靜摩擦死區）
static const double KD_MOTOR  = 0.0;    // 編碼器在低 RPM 量化粗，微分只會放大噪音 → 關閉
static const float  CPR       = 624.0;  // 每轉脈衝數 (13 齒 × 48 減速)
static const int    SAMPLE_MS = 50;     // PID 取樣週期 (ms)，20Hz（高 CPR + 低 RPM 需較長週期才有量測解析度）

// =====================================================================
// 內部狀態（外部不要直接存取）
// =====================================================================
static volatile long m_pulseL = 0, m_pulseR = 0;
static double m_targetL = 0, m_targetR = 0;
static double m_intgL   = 0, m_intgR   = 0;
static double m_prevEL  = 0, m_prevER  = 0;
static bool   m_fwdL    = true, m_fwdR  = true;
static unsigned long m_lastTick = 0;

// 中斷服務常式
static void m_isrL() { if (digitalRead(M_ENCB_L) == HIGH) m_pulseL++; else m_pulseL--; }
static void m_isrR() { if (digitalRead(M_ENCB_R) == LOW)  m_pulseR++; else m_pulseR--; }

// 內部：單輪 PID → PWM 
static int m_calcPWM(double tgt, double meas, double &intg, double &prevErr) {
  const double dt = SAMPLE_MS / 1000.0;
  double err   = tgt - meas;
  double trial = intg + err * dt;
  double deriv = (err - prevErr) / dt;

  double out = FF_MOTOR * tgt
             + KP_MOTOR * err
             + KI_MOTOR * trial
             + KD_MOTOR * deriv;

  if (out >= 0.0 && out <= 255.0) intg = trial; 
  prevErr = err;
  return (int)constrain(out, 0, 255);
}

// =====================================================================
// 公開 API
// =====================================================================
void motorBegin() {
  pinMode(M_ENA, OUTPUT); pinMode(M_IN1, OUTPUT); pinMode(M_IN2, OUTPUT);
  pinMode(M_ENB, OUTPUT); pinMode(M_IN3, OUTPUT); pinMode(M_IN4, OUTPUT);
  pinMode(M_ENCA_L, INPUT_PULLUP); pinMode(M_ENCB_L, INPUT_PULLUP);
  pinMode(M_ENCA_R, INPUT_PULLUP); pinMode(M_ENCB_R, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(M_ENCA_L), m_isrL, RISING);
  attachInterrupt(digitalPinToInterrupt(M_ENCA_R), m_isrR, RISING);
  motorStop();
}

// 設定目標轉速（單位：RPM；負值代表反轉）
void motorSetSpeed(double rpmL, double rpmR) {
  m_targetL = rpmL;
  m_targetR = rpmR;
}

void motorStop() {
  analogWrite(M_ENA, 0);
  analogWrite(M_ENB, 0);
  m_targetL = m_targetR = 0;
  m_intgL = m_intgR = 0;
  m_prevEL = m_prevER = 0;
}

// 在 loop() 每次呼叫；內部自動依 SAMPLE_MS 節流
void motorTick() {
  unsigned long now = millis();
  if (now - m_lastTick < (unsigned long)SAMPLE_MS) return;
  m_lastTick = now;

  // 取編碼器計數
  noInterrupts();
  long cntL = m_pulseL; m_pulseL = 0;
  long cntR = m_pulseR; m_pulseR = 0;
  interrupts();

  // 換向偵測 → 重置積分
  bool fwdL = (m_targetL >= 0);
  bool fwdR = (m_targetR >= 0);
  if (fwdL != m_fwdL) { m_intgL = 0; m_prevEL = 0; m_fwdL = fwdL; }
  if (fwdR != m_fwdR) { m_intgR = 0; m_prevER = 0; m_fwdR = fwdR; }

  // 方向腳設定
  digitalWrite(M_IN3, fwdL ? HIGH : LOW);
  digitalWrite(M_IN4, fwdL ? LOW  : HIGH);
  digitalWrite(M_IN1, fwdR ? LOW  : HIGH);
  digitalWrite(M_IN2, fwdR ? HIGH : LOW);

  // 編碼器 → 實測 RPM
  double rpmL = (double)cntL / CPR * (60000.0 / SAMPLE_MS);
  double rpmR = (double)cntR / CPR * (60000.0 / SAMPLE_MS);

  // 跑 PID（以絕對值算，方向已由腳位控制）
  int pwmL = m_calcPWM(abs(m_targetL), abs(rpmL), m_intgL, m_prevEL);
  int pwmR = m_calcPWM(abs(m_targetR), abs(rpmR), m_intgR, m_prevER);

  analogWrite(M_ENB, pwmL);
  analogWrite(M_ENA, pwmR);
}

// =====================================================================
// 高層動作 API
// 以 motorSetSpeed / motorStop 為基礎，speed 單位同樣是 RPM。
// =====================================================================
void Forward(double speed)   { motorSetSpeed( speed,  speed); }  // 兩輪同向前進
void Backward(double speed)  { motorSetSpeed(-speed, -speed); }  // 兩輪同向後退
void SpinLeft(double speed)  { motorSetSpeed(-speed,  speed); }  // 原地左轉（左輪後退、右輪前進）
void SpinRight(double speed) { motorSetSpeed( speed, -speed); }  // 原地右轉（左輪前進、右輪後退）
void Stop()                  { motorStop(); }
