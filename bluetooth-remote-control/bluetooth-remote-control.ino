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
 
  // 馬達初始化
  motorSetup();
}

void loop() {
  if (BT.available() > 0) {
    command = BT.read();

    // 自動轉大寫 (容錯小寫輸入)
    if (command >= 'a' && command <= 'z') {
      command = command - 'a' + 'A';
    }
    
    // 1. 速度指令 (字元 '0' 或 '1' 到 '9')
    if (command == '0') {
      speedVal = 0;
      updateMovement();
      
    } else if (command >= '1' && command <= '9') {
      int speedLevel = command - '0'; 
      speedVal = map(speedLevel, 1, 9, 100, 200); 
      updateMovement();
    }

    // 2. 方向指令 ('F', 'B', 'L', 'R', 'S')
    else if (command == 'F' || command == 'B' || command == 'L' || command == 'R' || command == 'S') {
      currentAction = command;
      updateMovement();
    }
  }
}
