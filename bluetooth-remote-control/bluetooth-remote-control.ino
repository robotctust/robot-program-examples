#include <SoftwareSerial.h>

const int BT_RX = 7; // Arduino 接收腳 (接 BT06 的 TXD)
const int BT_TX = 2; // Arduino 發送腳 (接 BT06 的 RXD)
SoftwareSerial BT(BT_RX, BT_TX);

char command;               // 指令碼
int speedVal = 100;         // 預設速度 (100~200)
char currentAction = 'S';   // 目前行進狀態 ('F', 'B', 'L', 'R', 'S')

// 依據目前狀態與速度更新馬達輸出
void updateMovement() {
  if (speedVal == 0 || currentAction == 'S') {
    Stop();
    return;
  }
  switch (currentAction) {
    case 'F': 
      Forward(speedVal);   
      break; 
    case 'B': 
      Backward(speedVal);  
      break; 
    case 'L': 
      SpinLeft(speedVal);  
      break; 
    case 'R': 
      SpinRight(speedVal); 
      break; 
    default:  
      Stop();              
      break; 
  }
}

void setup() {
  // 啟動 BT06 軟體序列埠
  BT.begin(9600);
  BT.setTimeout(30);
 
  // 馬達初始化
  motorSetup();
}

void loop() {
  if (BT.available() > 0) {
    command = BT.read();

    // 1. 差速混控指令 (M<L>,<R>\n，支援 iOS 與 Android 類比搖桿連續差速)
    if (command == 'M' || command == 'm') {
      String line = BT.readStringUntil('\n');
      line.trim();
      int commaIndex = line.indexOf(',');
      if (commaIndex != -1) {
        int leftVal = line.substring(0, commaIndex).toInt();
        int rightVal = line.substring(commaIndex + 1).toInt();
        DriveDifferential(leftVal, rightVal);
        if (leftVal == 0 && rightVal == 0) {
          currentAction = 'S';
        } else {
          currentAction = 'D';
        }
      }
    }
    // 2. 轉速/速度設定指令 (V<rpm>\n，相容 iOS 圓盤速度設定)
    else if (command == 'V' || command == 'v') {
      int speed = BT.parseInt();
      if (speed >= 0 && speed <= 255) {
        speedVal = speed;
        if (currentAction != 'S' && currentAction != 'D') {
          updateMovement();
        }
      }
    }
    // 3. 速度等級指令 (字元 '0' 或 '1' 到 '9')
    else if (command == '0') {
      speedVal = 0;
      updateMovement();
    } else if (command >= '1' && command <= '9') {
      int speedLevel = command - '0'; 
      speedVal = map(speedLevel, 1, 9, 100, 200); 
      updateMovement();
    }
    // 4. 方向指令 ('F', 'B', 'L', 'R', 'S'，容錯大小寫)
    else {
      if (command >= 'a' && command <= 'z') {
        command = command - 'a' + 'A';
      }
      if (command == 'F' || command == 'B' || command == 'L' || command == 'R' || command == 'S') {
        currentAction = command;
        updateMovement();
      }
    }
  }
}
