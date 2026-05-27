// =====================================================================
// A01 機器人循跡挑戰 — 主程式 (main.ino)
// =====================================================================

// Uno R4：用硬體 UART Serial1 (D0=RX / D1=TX) 接 HC-08。
// 接線：HC-08 TX → D0，HC-08 RX → D1。
int speed = 25;          // 預設速度（RPM，範圍 0–50）
const int SPEED_MAX = 50;

// 速度指令解析狀態：>= 0 代表正在收集 "V" 之後的數字
int spCollect = -1;

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
//   方向：單字元 F/B/L/R/S
//   速度：V<rpm>\n（例如 "V35\n"），rpm 0–50
// =====================================================================
void handleByte(char c) {
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
    case 'V': spCollect = 0;       break; // 進入速度收集模式
    case 'F': Forward(speed);      break; // 前進
    case 'B': Backward(speed);     break; // 後退
    case 'L': SpinLeft(speed);     break; // 左轉
    case 'R': SpinRight(speed);    break; // 右轉
    case 'S': Stop();              break; // 停止
  }
}
