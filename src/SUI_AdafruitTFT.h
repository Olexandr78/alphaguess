#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>

// Optional thin helper for displays driven by an Adafruit_GFX-compatible
// controller library (for example Adafruit_ST7735 or Adafruit_ILI9341).
// Include the controller's own header and pass its display object here.
namespace SUI {

template <typename Display>
class AdafruitTFT {
public:
  explicit AdafruitTFT(Display &display) : _display(display) {}

  void begin() {
    _display.fillScreen(0x0000);
    _display.setTextWrap(true);
  }

  void clear(uint16_t color = 0x0000) {
    _display.fillScreen(color);
  }

  void text(int16_t x, int16_t y, const char *value,
            uint16_t color = 0xFFFF, uint8_t size = 1) {
    _display.setCursor(x, y);
    _display.setTextColor(color);
    _display.setTextSize(size);
    _display.print(value);
  }

  void pixel(int16_t x, int16_t y, uint16_t color) {
    _display.drawPixel(x, y, color);
  }

  void rectangle(int16_t x, int16_t y, int16_t w, int16_t h,
                 uint16_t color) {
    _display.drawRect(x, y, w, h, color);
  }

  void filledRectangle(int16_t x, int16_t y, int16_t w, int16_t h,
                       uint16_t color) {
    _display.fillRect(x, y, w, h, color);
  }

private:
  Display &_display;
};

} // namespace SUI
