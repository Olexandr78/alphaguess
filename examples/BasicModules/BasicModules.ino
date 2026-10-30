#include <SashaUniversalIO.h>

SUI::Relay relay(7, SUI::ActiveLevel::LOW_LEVEL); // Many relay boards are active LOW
SUI::Button button(2);                      // Button between pin 2 and GND
SUI::Buzzer buzzer(8);
SUI::HCSR04 sonar(10, 11);

void setup() {
  Serial.begin(9600);
  relay.begin();
  button.begin();
  buzzer.begin();
  sonar.begin();
}

void loop() {
  if (button.isPressed()) {
    relay.toggle();
    buzzer.beep(1400, 80);
    delay(250); // Simple example debounce; use a state machine in production.
  }

  const float cm = sonar.distanceCm();
  if (cm >= 0) {
    Serial.print("Distance: ");
    Serial.print(cm, 1);
    Serial.println(" cm");
  } else {
    Serial.println("No echo");
  }

  delay(100);
}
