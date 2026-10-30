#include <SUI_I2CLCD.h>

SUI::I2CLCD lcd(0x27, 16, 2); // Try 0x3F if your backpack uses that address

void setup() {
  lcd.begin();
  lcd.setCursor(0, 0);
  lcd.print("SashaUniversalIO");
  lcd.setCursor(0, 1);
  lcd.print("I2C LCD ready");
}

void loop() {
}
