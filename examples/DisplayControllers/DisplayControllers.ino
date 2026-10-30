#include <SashaUniversalIO.h>

SUI::TM1637Display tm(2, 3);
SUI::MAX7219Display matrix(4, 5, 6);
SUI::StepDirStepper motor(7, 8, 9); // STEP, DIR, ENABLE
SUI::ShiftRegister595 outputs(11, 12, 13); // DATA, CLOCK, LATCH

void setup() {
  tm.begin(); tm.setBrightness(5);
  matrix.begin(4);
  motor.begin(); motor.enable();
  outputs.begin();
}

void loop() {
  static int value = 0;
  tm.showNumber(value);
  matrix.setDigit(0, (uint8_t)(value % 10));
  outputs.write((uint8_t)value);
  motor.move(200);
  value = (value + 1) % 10000;
  delay(250);
}
