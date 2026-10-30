#include <SUI_Servo.h>

SUI::ServoMotor arm(9);

void setup() {
  arm.begin();
}

void loop() {
  arm.writeAngle(0);
  delay(700);
  arm.writeAngle(90);
  delay(700);
  arm.writeAngle(180);
  delay(700);
}
