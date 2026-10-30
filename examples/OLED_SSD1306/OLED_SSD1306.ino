#define SUI_ENABLE_OLED
#include <SashaUniversalIO.h>

SUI::AdafruitOLED oled(128, 64, -1);
void setup() {
  if (!oled.begin(0x3C)) while (true) {}
  oled.clear();
  oled.text(0, 0, "SashaUniversalIO");
  oled.text(0, 16, "SSD1306 OLED");
  oled.flush();
}
void loop() {}
