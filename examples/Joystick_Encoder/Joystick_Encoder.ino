#include <SashaUniversalIO.h>

// HW-504: VRx=A0, VRy=A1, SW=D2
SUI::Joystick joystick(A0, A1, 2);
// HW-040: CLK=D3, DT=D4, SW=D5
SUI::RotaryEncoder encoder(3, 4, 5);
SUI::Relay relay(7, SUI::ActiveLevel::LOW);

void setup() {
  Serial.begin(9600);
  joystick.begin(); encoder.begin(); relay.begin();
}
void loop() {
  encoder.update();
  Serial.print("X="); Serial.print(joystick.x());
  Serial.print(" Y="); Serial.print(joystick.y());
  Serial.print(" encoder="); Serial.println(encoder.position());
  if (joystick.pressed()) relay.toggle();
  delay(5);
}
