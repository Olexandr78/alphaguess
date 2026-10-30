#include <SashaUniversalIO.h>

const uint8_t segPins[8] = {2,3,4,5,6,7,8,9};
SUI::SevenSegment display(segPins, false); // false = common cathode
SUI::Stepper4Wire motor(10,11,12,13,3);
const uint8_t dipPins[3] = {A0,A1,A2};
SUI::DipSwitch switches(dipPins, 3);
SUI::AnalogMicrophone mic(A3);
SUI::SoundDetector sound(A4);
SUI::ActiveBeeper beeper(A5);

void setup() {
  Serial.begin(9600);
  display.begin(); motor.begin(); switches.begin(); mic.begin(); sound.begin(); beeper.begin();
}
void loop() {
  display.digit(switches.value() % 10);
  Serial.print("DIP="); Serial.print(switches.value());
  Serial.print(" mic peak-to-peak="); Serial.print(mic.peakToPeak(20));
  Serial.print(" sound detected="); Serial.println(sound.detected());
  if (switches.read(0)) { motor.step(8); beeper.beep(30); }
  else motor.release();
  delay(20);
}
