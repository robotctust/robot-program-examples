// ==============================
// 基礎循線：直線與曲線
// ==============================

// 感測器腳位
const int S_FAR_LEFT  = 2;   // 最左（保留）
const int S_LEFT      = 3;
const int S_MID       = 4;
const int S_RIGHT     = 7;
const int S_FAR_RIGHT = 11;  // 最右（保留）

// 馬達腳位
int MR_A = 5;
int MR_B = 6;
int ML_A = 9;
int ML_B = 10;

// 速度參數
const int SPEED_FORWARD = 100;
const int SPEED_TURN    = 100;

// 馬達平衡校正
const int RIGHT_MOTOR_PCT = 100;
const int LEFT_MOTOR_PCT  = 100;

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
}

void loop() {
  bool stoped = false;
  int mid   = digitalRead(S_MID);
  int left  = digitalRead(S_LEFT);
  int right = digitalRead(S_RIGHT);
  int farLeft = digitalRead(S_FAR_LEFT);
  int farRight = digitalRead(S_FAR_RIGHT);
  int onStop = mid == 1 & left == 1 & right == 1 & farLeft == 1 & farRight == 1;

  if (stoped) {
    return;
  }

  if (mid == 1) {
    Forward(SPEED_FORWARD);
  } else if (left == 1) {
    SpinLeft(SPEED_TURN);
  } else if (right == 1) {
    SpinRight(SPEED_TURN);
  } else if (farLeft) {
    SpinLeft(SPEED_TURN);
  } else if (farRight) {
    SpinRight(SPEED_TURN);
  } else if (onStop) {
    Stop();
    stoped = true;
  } else {
    Stop();
  }

  Serial.print(mid);
}

// ── 底層馬達控制 ──

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

// ── 高層動作函式 ─-

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

void Stop() {
  digitalWrite(MR_A, LOW);
  digitalWrite(MR_B, LOW);
  digitalWrite(ML_A, LOW);
  digitalWrite(ML_B, LOW);
}