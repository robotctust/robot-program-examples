// =====================================================================
// 主程式 (main.ino)
// =====================================================================

int speed = 25; // 預設速度
const int SPEED_MAX = 50; // 最高速度

// 速度指令解析狀態：>= 0 代表正在收集 "V" 之後的數字
int spCollect = -1;

// 搖桿混控指令解析狀態：>= 0 代表正在收集 "M" 之後的 "<L>,<R>"
char mbuf[16];
int  mlen = -1;

// =====================================================================
// setup
// =====================================================================
void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);
  motorBegin();
}

// =====================================================================
// loop
// =====================================================================
void loop() {
  motorTick();    // 馬達 PID 內部自動節流

  while (Serial1.available() > 0) {
    handleByte((char)Serial1.read());
  }
}

// =====================================================================
// 指令處理
//   方向：單字元 F/B/L/R/S（圓盤模式，速度用內部預設 speed）
//   速度：V<rpm>\n（例如 "V35\n"），rpm 0–50
//   搖桿：M<L>,<R>\n（例如 "M-30,42\n"），左右輪轉速 -50..50，"M0,0" 為停車
// =====================================================================
void handleByte(char c) {
  // 正在收集搖桿混控數字
  if (mlen >= 0) {
    if (c == '\n' || c == '\r') {
      mbuf[mlen] = '\0';
      char *comma = strchr(mbuf, ',');
      if (comma) {
        *comma = '\0';
        int L = constrain(atoi(mbuf),     -SPEED_MAX, SPEED_MAX);
        int R = constrain(atoi(comma + 1), -SPEED_MAX, SPEED_MAX);
        if (L == 0 && R == 0) {
          Stop();
        } else {
          motorSetSpeed(L, R);
        }
        Serial.print("搖桿: L="); Serial.print(L);
        Serial.print(" R="); Serial.println(R);
      }
      mlen = -1;
      return;
    }
    if (mlen < (int)sizeof(mbuf) - 1) mbuf[mlen++] = c;
    return;
  }

  // 正在收集速度數字
  if (spCollect >= 0) {
    if (c >= '0' && c <= '9') {
      spCollect = spCollect * 10 + (c - '0');
      return;
    }
    // 遇到非數字（換行或其他）即結束並套用
    speed = constrain(spCollect, 0, SPEED_MAX);
    Serial.print("速度設定: ");
    Serial.println(speed);
    spCollect = -1;
    if (c == '\n' || c == '\r') return;   // 終止字元本身不再當指令
    // 其他字元繼續往下當方向指令處理
  }

  switch (c) {
    case 'M': mlen = 0;            break; // 進入搖桿混控收集模式
    case 'V': spCollect = 0;       break; // 進入速度收集模式
    case 'F': Forward(speed);      break; // 前進
    case 'B': Backward(speed);     break; // 後退
    case 'L': SpinLeft(speed);     break; // 左轉
    case 'R': SpinRight(speed);    break; // 右轉
    case 'S': Stop();              break; // 停止
  }
}
