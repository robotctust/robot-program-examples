// ── 馬達腳位 ──
int A_1A = 5;   // 左輪 PWM (ENA)
int A_1B = 6;   // 左輪 IN1
int B_1A = 9;   // 右輪 PWM (ENB)
int B_1B = 10;  // 右輪 IN2

// 馬達平衡校正（移植自 robot_basic_followline）
const int LEFT_MOTOR_PCT  = 100;
const int RIGHT_MOTOR_PCT = 100;

void motorSetup() {
  // 先清除輸出 latch，再切換方向，避免上電瞬間誤動作
  digitalWrite(A_1A, LOW);
  digitalWrite(A_1B, LOW);
  digitalWrite(B_1A, LOW);
  digitalWrite(B_1B, LOW);
  pinMode(A_1A, OUTPUT);
  pinMode(A_1B, OUTPUT);
  pinMode(B_1A, OUTPUT);
  pinMode(B_1B, OUTPUT);
}

// ── 底層馬達抽象（移植自 robot_basic_followline）──

void MotorLeft(bool fwd, int level) {
  int adj = (int)((long)level * LEFT_MOTOR_PCT / 100);
  analogWrite(A_1A, fwd ? adj : 0);
  analogWrite(A_1B, fwd ? 0   : adj);
}

void MotorRight(bool fwd, int level) {
  int adj = (int)((long)level * RIGHT_MOTOR_PCT / 100);
  analogWrite(B_1A, fwd ? adj : 0);
  analogWrite(B_1B, fwd ? 0   : adj);
}

// ── 高層動作函式（移植自 robot_basic_followline）──

void Forward(int level) {
  MotorLeft(true, level);
  MotorRight(true, level);
}

void Backward(int level) {
  MotorLeft(false, level);
  MotorRight(false, level);
}

void SpinLeft(int level) {
  MotorLeft(true, level);
  MotorRight(false, level);
}

void SpinRight(int level) {
  MotorLeft(false, level);
  MotorRight(true, level);
}

void Stop() {
  digitalWrite(A_1A, LOW);
  digitalWrite(A_1B, LOW);
  digitalWrite(B_1A, LOW);
  digitalWrite(B_1B, LOW);
}
