// Native SUI driver demo for ST7735S. No Adafruit libraries required.
#define SUI_ENABLE_ST7735S
#include <SashaUniversalIO.h>

#if defined(ARDUINO_ARCH_ESP32)
  // ESP32 VSPI defaults: SCK=18, MISO=19, MOSI=23
  #define TFT_CS  5
  #define TFT_DC  2
  #define TFT_RST 4
#else
  // Arduino Uno/Nano SPI defaults: SCK=13, MOSI=11
  #define TFT_CS  10
  #define TFT_DC  9
  #define TFT_RST 8
#endif

SUI::ST7735S tft(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  tft.begin(128, 160, 1); // 1 = landscape; use 0 for portrait

  const uint16_t blue   = SUI::ST7735S::color565(20, 60, 180);
  const uint16_t red    = SUI::ST7735S::color565(240, 40, 40);
  const uint16_t green  = SUI::ST7735S::color565(30, 220, 90);
  const uint16_t yellow = SUI::ST7735S::color565(255, 220, 0);
  const uint16_t white  = SUI::ST7735S::color565(255, 255, 255);

  tft.fillScreen(blue);
  tft.drawRect(3, 3, tft.width() - 6, tft.height() - 6, white);
  tft.fillRect(12, 14, 45, 25, red);
  tft.fillCircle(95, 27, 14, yellow);
  tft.drawLine(10, 58, tft.width() - 10, 58, green);

  tft.setCursor(12, 72);
  tft.setTextColor(white);
  tft.setTextSize(2);
  tft.println("SASHA SUI");

  tft.setTextSize(1);
  tft.setCursor(12, 94);
  tft.println("NATIVE ST7735S");
  tft.println("NO ADAFRUIT GFX");
}

void loop() {
}
