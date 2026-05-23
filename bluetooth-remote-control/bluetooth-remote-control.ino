#include <SoftwareSerial.h>

SoftwareSerial BT(12, 13);  // (RX, TX)
char command;

int speed = 100;

void setup() {
  BT.begin(9600);
  motorSetup();
}

void loop() {
  if (BT.available() > 0) {
    command = BT.read();
    switch (command) {
      case 'a': Forward(speed);   break;  // 前進
      case 'd': Backward(speed);  break;  // 後退
      case 'c': SpinLeft(speed);  break;  // 左轉
      case 'b': SpinRight(speed); break;  // 右轉
      case 's':                      // 停止
      default:  Stop();
    }
  }
}
