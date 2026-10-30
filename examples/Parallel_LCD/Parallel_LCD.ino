#include <SashaUniversalIO.h>

// RS, EN, D4, D5, D6, D7. Tie LCD R/W to GND.
SUI::ParallelLCD lcd(7, 8, 9, 10, 11, 12, 16, 2);
void setup() {
  lcd.begin();
  lcd.setCursor(0,0); lcd.print("SashaUniversalIO");
  lcd.setCursor(0,1); lcd.print("Parallel LCD");
}
void loop() {}
