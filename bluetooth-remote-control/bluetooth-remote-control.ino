#include <SoftwareSerial.h>

SoftwareSerial BT(13, 12);  // (RX, TX)
char command; // 指令碼
int speed = 100; // 預設速度

void setup() {
  Serial.begin(115200);
  BT.begin(9600);
  
  // 馬達初始化
  motorSetup();
}

void loop() {
  if (BT.available() > 0) {
    command = BT.read();
    
    // 1. 判斷是否為速度指令 (字元 '0' 或 '1' 到 '9')
    if (command == '0') {
      speed = 0;
      Serial.print("目前速度: ");
      Serial.println(speed);
      
    } else if (command >= '1' && command <= '9') {
      int speedLevel = command - '0'; 
      speed = map(speedLevel, 1, 9, 100, 200); 
      Serial.print("目前速度: ");
      Serial.println(speed);
    }

    // 2. 判斷是否為方向指令 ('a', 'b', 'c', 'd', 's')
    else {
      switch (command) {
        case 'a': Forward(speed);   break; // 前進
        case 'd': Backward(speed);  break; // 後退
        case 'c': SpinLeft(speed);  break; // 左轉
        case 'b': SpinRight(speed); break; // 右轉
        case 's': Stop();           break; // 停止
      }
    }
  }
}
