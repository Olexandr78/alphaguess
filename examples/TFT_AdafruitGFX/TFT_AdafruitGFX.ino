// Example for an Adafruit_GFX-compatible TFT.
// Install the controller library matching your display first.
// This sketch is intentionally a template: constructor and pins differ
// between ST7735, ST7789, ILI9341 and other controller boards.

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <SUI_AdafruitTFT.h>

#define TFT_CS   10
#define TFT_DC    9
#define TFT_RST   8

Adafruit_ST7789 display(TFT_CS, TFT_DC, TFT_RST);
SUI::AdafruitTFT<Adafruit_ST7789> screen(display);

void setup() {
  display.init(240, 320); // Change to the actual panel resolution.
  display.setRotation(1);
  screen.begin();
  screen.text(8, 8, "SashaUniversalIO", 0x07E0, 2);
  screen.text(8, 36, "Adafruit_GFX adapter", 0xFFFF, 1);
}

void loop() {
}
