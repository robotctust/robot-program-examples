// ==========================================
// 進階循線：七階查表式循線 + 地圖驅動路口決策
// ==========================================

// ===== 感測器腳位 =====
const int S_FAR_LEFT  = 2;
const int S_LEFT      = 3;
const int S_MID       = 4;
const int S_RIGHT     = 7;
const int S_FAR_RIGHT = 11;

// ===== 馬達腳位 =====
const int MR_A = 5;
const int MR_B = 6;
const int ML_A = 9;
const int ML_B = 10;

// ===== 感測器邏輯 =====
const int SENSOR_BLACK = HIGH;   // 感測到黑線時的電位（顛倒則改 LOW）

// ===== 速度參數 =====
const int SPEED_FORWARD = 110;   // 直走速度
const int SPEED_CORRECT =  80;   // 極偏時的旋轉速度
const int SPEED_TURN    = 100;   // 路口轉彎速度
const int SPEED_GENTLE  =  55;   // 七階循線：輕微修正的慢側速度
const int SPEED_STRONG  =  25;   // 七階循線：強力修正的慢側速度（反轉）

// ===== 馬達平衡校正（請根據你的車體自行微調）=====
const int RIGHT_MOTOR_PCT = 100;
const int LEFT_MOTOR_PCT  = 100;

// ===== 路口轉彎時間（ms）=====
//   盲轉期間使用固定時間旋轉，之後靠感測器尋線完成。
//   實機如果發現轉不準，先量 90° 再線性外推 45° / 135°。
const unsigned long TURN_45_MS  = 125;
const unsigned long TURN_90_MS  = 275;
const unsigned long TURN_135_MS = 450;

// ===== 路口冷卻 =====
//   觸發路口後此時間內不再觸發，避免連續誤判。
const unsigned long JUNC_CD_MS = 500;

// ===== 虛線空白容忍 =====
//   連續多少個 loop 沒有任何感測器亮起，才從「沿用上次修正」改為「直走」。
//   數值太小：輕微顛簸就切成直走，失去方向記憶。
//   數值太大：激進修正會通過整段空隙，造成偏移。
const int DASH_GAP_CYCLES = 20;

// === 感測器模式（位元化）==================================================================
// bit0 = FAR_LEFT   bit1 = LEFT   bit2 = MID   bit3 = RIGHT   bit4 = FAR_RIGHT
#define P_L   (1 << 0)
#define P_ML  (1 << 1)
#define P_M   (1 << 2)
#define P_MR  (1 << 3)
#define P_R   (1 << 4)

// 感測器模式
#define PAT_LEFT3   (P_L  | P_ML | P_M)               // [FAR_LEFT, LEFT, MID]          左 T 字
#define PAT_RIGHT3  (P_M  | P_MR | P_R)               // [MID, RIGHT, FAR_RIGHT]        右 T 字
#define PAT_ML_M    (P_ML | P_M)                      // [LEFT, MID]                    窄角左路主觸發
#define PAT_L_M     (P_L  | P_M)                      // [FAR_LEFT, MID]                窄角左路預觸發
#define PAT_DOT_R   (P_M  | P_R)                      // [MID, FAR_RIGHT]               虛線右路
#define PAT_R_M     (P_MR | P_R)                      // [RIGHT, FAR_RIGHT]             窄角右路預觸發
#define PAT_CTR     (P_ML | P_M | P_MR)               // 中央區（M亮且兩側外感測器未亮）
#define PAT_ALL     (P_L  | P_ML | P_M | P_MR | P_R)  // [全部]                         十字 / 終點

// === 動作列舉與地圖結構 ==================================================================
enum Action {
  ACT_LEFT_45,
  ACT_RIGHT_45,
  ACT_LEFT_90,
  ACT_RIGHT_90,
  ACT_LEFT_135,
  ACT_RIGHT_135,
  ACT_END,
};

struct MapStep {
  uint8_t prePattern;  // 預觸發模式（0 = 無需預觸發，直接偵測 pattern）
  uint8_t pattern;     // 主觸發模式
  Action  action; 
};

// === 地圖定義 ==================================================================
// 此版本根據 AERC A 圖所寫
const MapStep MAP[] = {
//  prePattern  pattern      action
  { 0,          PAT_RIGHT3,  ACT_RIGHT_90  },   //  1. 右轉 90°
  { 0,          PAT_RIGHT3,  ACT_RIGHT_90  },   //  2. 右轉 90°
  { 0,          PAT_LEFT3,   ACT_LEFT_90   },   //  3. 左轉 90°
  { 0,          PAT_LEFT3,   ACT_LEFT_90   },   //  4. 左轉 90°
  { 0,          PAT_RIGHT3,  ACT_RIGHT_90  },   //  5. 右轉 90°
  { 0,          PAT_RIGHT3,  ACT_RIGHT_90  },   //  6. 右轉 90°
  { PAT_L_M,    PAT_ML_M,    ACT_LEFT_135  },   //  7. 左轉 135°（先見 [FAR_LEFT,MID]，再見 [LEFT,MID] 才觸發）
  { PAT_R_M,    PAT_CTR,     ACT_LEFT_45   },   //  8. 左轉 45°（先見 [RIGHT,FAR_RIGHT]，再見中央態才觸發）
  { 0,          PAT_RIGHT3,  ACT_RIGHT_90  },   //  9. 右轉 90°
  { 0,          PAT_RIGHT3,  ACT_RIGHT_90  },   // 10. 右轉 90°
  { 0,          PAT_DOT_R,   ACT_RIGHT_90  },   // 11. 虛線右路右轉 90°
  { 0,          PAT_LEFT3,   ACT_LEFT_90   },   // 12. 左轉 90°
  { 0,          PAT_LEFT3,   ACT_LEFT_90   },   // 13. 左轉 90°
  { 0,          PAT_DOT_R,   ACT_RIGHT_90  },   // 14. 虛線右路右轉 90°
  { 0,          PAT_RIGHT3,  ACT_RIGHT_90  },   // 15. 右轉 90°
  { 0,          PAT_LEFT3,   ACT_LEFT_90   },   // 16. 左轉 90°
  { 0,          PAT_LEFT3,   ACT_LEFT_90   },   // 17. 左轉 90°
  { 0,          PAT_RIGHT3,  ACT_RIGHT_90  },   // 18. 右轉 90°
  { 0,          PAT_ALL,     ACT_LEFT_90   },   // 19. 十字路口左轉 90°（五感全黑）
  { 0,          PAT_ALL,     ACT_END       },   // 20. 終點停止
};
const int N_MAP = sizeof(MAP) / sizeof(MAP[0]);

// === 內部狀態 ==================================================================
bool          robot_done  = false;
int           juncIdx     = 0;       // 目前等待觸發的路口（= MAP 索引）
bool          juncPreSeen = false;   // 預觸發旗標
unsigned long lastJuncT   = 0;       // 上次路口觸發時間（冷卻用）
int           blankCount  = 0;       // 連續無感測器亮起的計數（虛線空白容忍用）

// 七階循線
enum DriveMode {
  MODE_FORWARD,   // 置中直走
  MODE_GENTLE_L,  // 輕微左修（輕右偏矯正）
  MODE_GENTLE_R,  // 輕微右修（輕左偏矯正）
  MODE_STRONG_L,  // 強力左修（強右偏矯正）
  MODE_STRONG_R,  // 強力右修（強左偏矯正）
  MODE_SPIN_L,    // 原地左旋（極右偏矯正）
  MODE_SPIN_R,    // 原地右旋（極左偏矯正）
};
// 記憶上次驅動模式
static DriveMode lastMode = MODE_FORWARD;

// === 感測器模式比對 ==================================================================
static bool patternMatches(uint8_t expected, bool bL, bool bML, bool bM, bool bMR, bool bR) {
  switch (expected) {
    case PAT_LEFT3:  return  bL && bML && bM && !bR;
    case PAT_RIGHT3: return !bL && bM  && bMR && bR;
    case PAT_ML_M:   return !bL && (bML || bM) && !bR;
    case PAT_L_M:    return  bL && (bML || bM) && !bMR && !bR;
    case PAT_DOT_R:  return  bM && bR;
    case PAT_R_M:    return  bR && (bMR || bM) && !bML && !bL;
    case PAT_CTR:    return  bM && !bL && !bR;
    case PAT_ALL:    return  bL && bML && bM && bMR && bR;
    default:         return false;
  }
}

// === 動作名稱（除錯訊息用）==================================================================
static const char* actionName(Action a) {
  switch (a) {
    case ACT_LEFT_45:   return "L45";
    case ACT_RIGHT_45:  return "R45";
    case ACT_LEFT_90:   return "L90";
    case ACT_RIGHT_90:  return "R90";
    case ACT_LEFT_135:  return "L135";
    case ACT_RIGHT_135: return "R135";
    case ACT_END:       return "END";
  }
  return "?";
}

// === setup ==================================================================
void setup() {
  Serial.begin(115200);

  pinMode(S_FAR_LEFT,  INPUT);
  pinMode(S_LEFT,      INPUT);
  pinMode(S_MID,       INPUT);
  pinMode(S_RIGHT,     INPUT);
  pinMode(S_FAR_RIGHT, INPUT);

  pinMode(MR_A, OUTPUT);
  pinMode(MR_B, OUTPUT);
  pinMode(ML_A, OUTPUT);
  pinMode(ML_B, OUTPUT);

  Serial.print("=== 進階循線啟動，地圖共 ");
  Serial.print(N_MAP);
  Serial.println(" 個路口 ===");

  delay(1000);
}

// === loop ==================================================================
void loop() {
  if (robot_done) {
    stop();
    return;
  }

  // 讀感測器（true = 碰到黑線）
  bool bL  = (digitalRead(S_FAR_LEFT)  == SENSOR_BLACK);
  bool bML = (digitalRead(S_LEFT)      == SENSOR_BLACK);
  bool bM  = (digitalRead(S_MID)       == SENSOR_BLACK);
  bool bMR = (digitalRead(S_RIGHT)     == SENSOR_BLACK);
  bool bR  = (digitalRead(S_FAR_RIGHT) == SENSOR_BLACK);

  unsigned long now = millis();

  // 路口偵測（地圖驅動，支援預觸發 + 冷卻）
  if (juncIdx < N_MAP && (now - lastJuncT > JUNC_CD_MS)) {
    const MapStep step = MAP[juncIdx];

    if (step.prePattern != 0 && !juncPreSeen) {
      // 第一段：等待預觸發模式
      if (patternMatches(step.prePattern, bL, bML, bM, bMR, bR)) {
        juncPreSeen = true;
        Serial.print(">>> 路口 #");
        Serial.print(juncIdx + 1);
        Serial.println(" 預觸發");
      }
    } else {
      // 第二段：預觸發已滿足（或不需要），等待主觸發模式
      if (patternMatches(step.pattern, bL, bML, bM, bMR, bR)) {
        Serial.print(">>> 路口 #");
        Serial.print(juncIdx + 1);
        Serial.print(" / ");
        Serial.print(N_MAP);
        Serial.print("  動作=");
        Serial.println(actionName(step.action));
        lastJuncT   = now;
        juncPreSeen = false;
        juncIdx++;
        blankCount  = 0;
        performAction(step.action);
        return;
      }
    }
  }

  // 七階查表循線
  bool anyBlack = bL || bML || bM || bMR || bR;
  DriveMode mode;

  if (!anyBlack) {
    // 空白段：短空隙沿用上次修正，長空隙改直走
    blankCount++;
    mode = (blankCount > DASH_GAP_CYCLES) ? MODE_FORWARD : lastMode;
  } else {
    blankCount = 0;
    if      (!bML &&  bM  && !bMR        )  mode = MODE_FORWARD;   // 置中
    else if ( bML &&  bM  && !bMR        )  mode = MODE_GENTLE_L;  // 輕右偏
    else if (!bML &&  bM  &&  bMR        )  mode = MODE_GENTLE_R;  // 輕左偏
    else if ( bML && !bM  && !bMR        )  mode = MODE_STRONG_L;  // 強右偏
    else if (!bML && !bM  &&  bMR        )  mode = MODE_STRONG_R;  // 強左偏
    else if ( bL  && !bML && !bM && !bMR )  mode = MODE_SPIN_L;    // 極右偏
    else if (!bML && !bM  && !bMR &&  bR )  mode = MODE_SPIN_R;    // 極左偏
    else if ( bML &&  bM  &&  bMR        )  mode = MODE_FORWARD;   // 橫線：直行穿越
    else                                     mode = lastMode;      // 未知態：沿用
  }

  lastMode = mode;
  applyDriveMode(mode);
}

// === 路口轉彎動作實作 ==================================================================
// 先盲轉固定時間（TURN_*_MS），再靠感測器尋回黑線。
// 注意：因感測器在前方，需要延遲盲彎機制讓旋轉軸心推到路口。
// 這裡的 delay 皆須依據您的車子速度、電池電量來手動微調。
void performAction(Action action) {
  const int forward_delay_ms = 200;  // 路口前推延遲（讓軸心對齊路口）

  switch (action) {
    case ACT_LEFT_90:
      Forward(SPEED_FORWARD);  delay(forward_delay_ms);
      SpinLeft(SPEED_TURN);    delay(TURN_90_MS);
      while (!(digitalRead(S_MID) == SENSOR_BLACK)) SpinLeft(SPEED_TURN);
      break;

    case ACT_LEFT_135:
      Forward(SPEED_FORWARD);  delay(forward_delay_ms);
      SpinLeft(SPEED_TURN);    delay(TURN_135_MS);
      while (!(digitalRead(S_MID) == SENSOR_BLACK)) SpinLeft(SPEED_TURN);
      break;

    case ACT_LEFT_45:
      // 45° 不需前推，感測器觸發時已有角度偏移
      SpinLeft(SPEED_TURN);    delay(TURN_45_MS);
      while (!(digitalRead(S_MID) == SENSOR_BLACK)) SpinLeft(SPEED_TURN);
      break;

    case ACT_RIGHT_90:
      Forward(SPEED_FORWARD);  delay(forward_delay_ms);
      SpinRight(SPEED_TURN);   delay(TURN_90_MS);
      while (!(digitalRead(S_MID) == SENSOR_BLACK)) SpinRight(SPEED_TURN);
      break;

    case ACT_RIGHT_135:
      Forward(SPEED_FORWARD);  delay(forward_delay_ms);
      SpinRight(SPEED_TURN);   delay(TURN_135_MS);
      while (!(digitalRead(S_MID) == SENSOR_BLACK)) SpinRight(SPEED_TURN);
      break;

    case ACT_RIGHT_45:
      SpinRight(SPEED_TURN);   delay(TURN_45_MS);
      while (!(digitalRead(S_MID) == SENSOR_BLACK)) SpinRight(SPEED_TURN);
      break;

    case ACT_END:
      robot_done = true;
      stop();
      Serial.println(">>> 抵達終點");
      return;
  }
  stop();
  delay(100);
}

// === 七階循線驅動模式執行 ==================================================================
void applyDriveMode(DriveMode mode) {
  switch (mode) {
    case MODE_FORWARD:   Forward(SPEED_FORWARD);                                            break;
    case MODE_GENTLE_L:  MotorRight(true,  SPEED_FORWARD); MotorLeft(true,  SPEED_GENTLE);  break;
    case MODE_GENTLE_R:  MotorRight(true,  SPEED_GENTLE);  MotorLeft(true,  SPEED_FORWARD); break;
    case MODE_STRONG_L:  MotorRight(true,  SPEED_FORWARD); MotorLeft(false, SPEED_STRONG);  break;
    case MODE_STRONG_R:  MotorRight(false, SPEED_STRONG);  MotorLeft(true,  SPEED_FORWARD); break;
    case MODE_SPIN_L:    SpinLeft(SPEED_CORRECT);                                           break;
    case MODE_SPIN_R:    SpinRight(SPEED_CORRECT);                                          break;
  }
}

// === 底層馬達控制 ==================================================================
void MotorRight(bool forward, int level) {
  int adj = (int)((long)level * RIGHT_MOTOR_PCT / 100);
  analogWrite(MR_A, forward ? adj : 0);
  analogWrite(MR_B, !forward ? adj : 0);
}

void MotorLeft(bool forward, int level) {
  int adj = (int)((long)level * LEFT_MOTOR_PCT / 100);
  analogWrite(ML_B, forward ? adj : 0);
  analogWrite(ML_A, !forward ? adj : 0);
}

// === 高層動作函式 ==================================================================
void Forward(int level) {
  MotorRight(true, level);
  MotorLeft(true, level);
}

void SpinLeft(int level) {
  MotorRight(true, level);
  MotorLeft(false, level);
}

void SpinRight(int level) {
  MotorRight(false, level);
  MotorLeft(true, level);
}

void stop() {
  digitalWrite(MR_A, LOW);
  digitalWrite(MR_B, LOW);
  digitalWrite(ML_A, LOW);
  digitalWrite(ML_B, LOW);
}
