#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <math.h>

// Native SPI driver for ST7735S panels. No Adafruit dependency.
namespace SUI {

class ST7735S : public Print {
public:
  explicit ST7735S(uint8_t cs, uint8_t dc, uint8_t rst,
                   SPIClass &spi = SPI)
      : _cs(cs), _dc(dc), _rst(rst), _spi(spi),
        _width(128), _height(160), _nativeWidth(128), _nativeHeight(160), _rotation(0),
        _cursorX(0), _cursorY(0), _textColor(0xFFFF),
        _textBg(0x0000), _textSize(1), _wrap(true),
        _xStart(0), _yStart(0) {}

  void begin(uint16_t width = 128, uint16_t height = 160,
             uint8_t rotation = 0) {
    _nativeWidth = width;
    _nativeHeight = height;
    _width = width;
    _height = height;
    _rotation = rotation & 3;

    pinMode(_cs, OUTPUT);
    pinMode(_dc, OUTPUT);
    pinMode(_rst, OUTPUT);
    digitalWrite(_cs, HIGH);
    digitalWrite(_dc, HIGH);
    _spi.begin();

    digitalWrite(_rst, HIGH); delay(5);
    digitalWrite(_rst, LOW);  delay(20);
    digitalWrite(_rst, HIGH); delay(150);

    command(0x01); delay(150); // SWRESET
    command(0x11); delay(120); // SLPOUT

    const uint8_t frame[] = {0x01, 0x2C, 0x2D};
    commandData(0xB1, frame, sizeof(frame));
    commandData(0xB2, frame, sizeof(frame));
    const uint8_t frame3[] = {0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D};
    commandData(0xB3, frame3, sizeof(frame3));
    const uint8_t inv[] = {0x07};
    commandData(0xB4, inv, sizeof(inv));
    const uint8_t power1[] = {0xA2, 0x02, 0x84};
    commandData(0xC0, power1, sizeof(power1));
    const uint8_t power2[] = {0xC5};
    commandData(0xC1, power2, sizeof(power2));
    const uint8_t power3[] = {0x0A, 0x00};
    commandData(0xC2, power3, sizeof(power3));
    const uint8_t power4[] = {0x8A, 0x2A};
    commandData(0xC3, power4, sizeof(power4));
    const uint8_t power5[] = {0x8A, 0xEE};
    commandData(0xC4, power5, sizeof(power5));
    const uint8_t vcom[] = {0x0E};
    commandData(0xC5, vcom, sizeof(vcom));

    const uint8_t positive[] = {
      0x02,0x1C,0x07,0x12,0x37,0x32,0x29,0x2D,
      0x29,0x25,0x2B,0x39,0x00,0x01,0x03,0x10
    };
    const uint8_t negative[] = {
      0x03,0x1D,0x07,0x06,0x2E,0x2C,0x29,0x2D,
      0x2E,0x2E,0x37,0x3F,0x00,0x00,0x02,0x10
    };
    commandData(0xE0, positive, sizeof(positive));
    commandData(0xE1, negative, sizeof(negative));

    const uint8_t colorMode[] = {0x05}; // RGB565
    commandData(0x3A, colorMode, sizeof(colorMode));
    setRotation(_rotation);
    command(0x13); delay(10);  // NORON
    command(0x29); delay(100); // DISPON
    fillScreen(0x0000);
  }

  uint16_t width() const { return _width; }
  uint16_t height() const { return _height; }

  void setRotation(uint8_t rotation) {
    _rotation = rotation & 3;
    uint8_t madctl = 0xC8;
    switch (_rotation) {
      case 0: madctl = 0xC8; _width = _nativeWidth; _height = _nativeHeight; break;
      case 1: madctl = 0xA8; _width = _nativeHeight; _height = _nativeWidth; break;
      case 2: madctl = 0x08; _width = _nativeWidth; _height = _nativeHeight; break;
      default: madctl = 0x68; _width = _nativeHeight; _height = _nativeWidth; break;
    }
    const uint8_t value[] = {madctl};
    commandData(0x36, value, 1);
  }

  void fillScreen(uint16_t color) {
    fillRect(0, 0, _width, _height, color);
  }

  void drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (x < 0 || y < 0 || x >= (int16_t)_width || y >= (int16_t)_height) return;
    setAddrWindow(x, y, x, y);
    writeColor(color);
    finishPixels();
  }

  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (w <= 0 || h <= 0 || x >= (int16_t)_width || y >= (int16_t)_height) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > _width) w = _width - x;
    if (y + h > _height) h = _height - y;
    if (w <= 0 || h <= 0) return;
    setAddrWindow(x, y, x + w - 1, y + h - 1);
    uint32_t count = (uint32_t)w * h;
    while (count--) writeColor(color);
    finishPixels();
  }

  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (w <= 0 || h <= 0) return;
    drawLine(x, y, x + w - 1, y, color);
    drawLine(x, y + h - 1, x + w - 1, y + h - 1, color);
    drawLine(x, y, x, y + h - 1, color);
    drawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
  }

  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int16_t error = dx + dy;
    for (;;) {
      drawPixel(x0, y0, color);
      if (x0 == x1 && y0 == y1) break;
      int16_t e2 = 2 * error;
      if (e2 >= dy) { error += dy; x0 += sx; }
      if (e2 <= dx) { error += dx; y0 += sy; }
    }
  }

  void drawCircle(int16_t x0, int16_t y0, int16_t radius, uint16_t color) {
    if (radius < 0) return;
    int16_t f = 1 - radius, ddx = 1, ddy = -2 * radius;
    int16_t x = 0, y = radius;
    drawPixel(x0, y0 + radius, color); drawPixel(x0, y0 - radius, color);
    drawPixel(x0 + radius, y0, color); drawPixel(x0 - radius, y0, color);
    while (x < y) {
      if (f >= 0) { --y; ddy += 2; f += ddy; }
      ++x; ddx += 2; f += ddx;
      drawPixel(x0 + x, y0 + y, color); drawPixel(x0 - x, y0 + y, color);
      drawPixel(x0 + x, y0 - y, color); drawPixel(x0 - x, y0 - y, color);
      drawPixel(x0 + y, y0 + x, color); drawPixel(x0 - y, y0 + x, color);
      drawPixel(x0 + y, y0 - x, color); drawPixel(x0 - y, y0 - x, color);
    }
  }

  void fillCircle(int16_t x0, int16_t y0, int16_t radius, uint16_t color) {
    if (radius < 0) return;
    for (int16_t y = -radius; y <= radius; ++y) {
      int16_t span = (int16_t)sqrt((double)radius * radius - (double)y * y);
      drawLine(x0 - span, y0 + y, x0 + span, y0 + y, color);
    }
  }

  void setCursor(int16_t x, int16_t y) { _cursorX = x; _cursorY = y; }
  void setTextColor(uint16_t color) { _textColor = color; _textBg = color; }
  void setTextColor(uint16_t color, uint16_t background) {
    _textColor = color; _textBg = background;
  }
  void setTextSize(uint8_t size) { _textSize = size ? size : 1; }
  void setTextWrap(bool wrap) { _wrap = wrap; }

  void text(int16_t x, int16_t y, const char *value,
            uint16_t color = 0xFFFF, uint8_t size = 1) {
    setCursor(x, y); setTextColor(color); setTextSize(size);
    if (value) print(value);
  }

  static uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) {
    return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
  }

  size_t write(uint8_t c) override {
    if (c == '\r') return 1;
    if (c == '\n') { _cursorX = 0; _cursorY += 8 * _textSize; return 1; }
    if (_wrap && (_cursorX + 6 * _textSize > (int16_t)_width)) {
      _cursorX = 0; _cursorY += 8 * _textSize;
    }
    if (_cursorY + 8 * _textSize > (int16_t)_height) _cursorY = 0;
    drawChar(_cursorX, _cursorY, (char)c, _textColor, _textBg, _textSize);
    _cursorX += 6 * _textSize;
    return 1;
  }

private:
  uint8_t _cs, _dc, _rst;
  SPIClass &_spi;
  uint16_t _width, _height;
  uint16_t _nativeWidth, _nativeHeight;
  uint8_t _rotation;
  int16_t _cursorX, _cursorY;
  uint16_t _textColor, _textBg;
  uint8_t _textSize;
  bool _wrap;
  uint8_t _xStart, _yStart;

  void command(uint8_t value) {
    _spi.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW); digitalWrite(_dc, LOW);
    _spi.transfer(value);
    digitalWrite(_cs, HIGH);
    _spi.endTransaction();
  }

  void commandData(uint8_t cmd, const uint8_t *data, size_t length) {
    _spi.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW); digitalWrite(_dc, LOW); _spi.transfer(cmd);
    digitalWrite(_dc, HIGH);
    for (size_t i = 0; i < length; ++i) _spi.transfer(data[i]);
    digitalWrite(_cs, HIGH);
    _spi.endTransaction();
  }

  void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    x0 += _xStart; x1 += _xStart; y0 += _yStart; y1 += _yStart;
    uint8_t data[4];
    _spi.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    digitalWrite(_dc, LOW); _spi.transfer(0x2A);
    digitalWrite(_dc, HIGH);
    data[0] = x0 >> 8; data[1] = x0; data[2] = x1 >> 8; data[3] = x1;
    for (uint8_t i = 0; i < 4; ++i) _spi.transfer(data[i]);
    digitalWrite(_dc, LOW); _spi.transfer(0x2B);
    digitalWrite(_dc, HIGH);
    data[0] = y0 >> 8; data[1] = y0; data[2] = y1 >> 8; data[3] = y1;
    for (uint8_t i = 0; i < 4; ++i) _spi.transfer(data[i]);
    digitalWrite(_dc, LOW); _spi.transfer(0x2C);
    digitalWrite(_dc, HIGH);
    // Keep CS asserted; pixel writes follow in the same SPI transaction.
    _windowOpen = true;
  }

  bool _windowOpen = false;

  void writeColor(uint16_t color) {
    if (!_windowOpen) return;
    _spi.transfer(color >> 8); _spi.transfer(color & 0xFF);
    // Pixel streams are terminated by the next command or transaction.
  }

  void finishPixels() {
    if (_windowOpen) {
      digitalWrite(_cs, HIGH);
      _spi.endTransaction();
      _windowOpen = false;
    }
  }

  void drawChar(int16_t x, int16_t y, char c, uint16_t color,
                uint16_t background, uint8_t size) {
    const uint8_t *glyph = glyphFor(c);
    for (uint8_t col = 0; col < 5; ++col) {
      uint8_t bits = pgm_read_byte(&glyph[col]);
      for (uint8_t row = 0; row < 7; ++row) {
        uint16_t pixelColor = (bits & (1 << row)) ? color : background;
        if (size == 1) drawPixel(x + col, y + row, pixelColor);
        else fillRect(x + col * size, y + row * size, size, size, pixelColor);
      }
    }
    if (size == 1) {
      for (uint8_t row = 0; row < 7; ++row) drawPixel(x + 5, y + row, background);
    } else {
      fillRect(x + 5 * size, y, size, 7 * size, background);
    }
  }

  static const uint8_t *glyphFor(char input) {
    static const uint8_t font[][5] PROGMEM = {
      {0x3E,0x51,0x49,0x45,0x3E}, {0x00,0x42,0x7F,0x40,0x00},
      {0x42,0x61,0x51,0x49,0x46}, {0x21,0x41,0x45,0x4B,0x31},
      {0x18,0x14,0x12,0x7F,0x10}, {0x27,0x45,0x45,0x45,0x39},
      {0x3C,0x4A,0x49,0x49,0x30}, {0x01,0x71,0x09,0x05,0x03},
      {0x36,0x49,0x49,0x49,0x36}, {0x06,0x49,0x49,0x29,0x1E},
      {0x7E,0x11,0x11,0x11,0x7E}, {0x7F,0x49,0x49,0x49,0x36},
      {0x3E,0x41,0x41,0x41,0x22}, {0x7F,0x41,0x41,0x22,0x1C},
      {0x7F,0x49,0x49,0x49,0x41}, {0x7F,0x09,0x09,0x09,0x01},
      {0x3E,0x41,0x49,0x49,0x7A}, {0x7F,0x08,0x08,0x08,0x7F},
      {0x00,0x41,0x7F,0x41,0x00}, {0x20,0x40,0x41,0x3F,0x01},
      {0x7F,0x08,0x14,0x22,0x41}, {0x7F,0x40,0x40,0x40,0x40},
      {0x7F,0x02,0x0C,0x02,0x7F}, {0x7F,0x04,0x08,0x10,0x7F},
      {0x3E,0x41,0x41,0x41,0x3E}, {0x7F,0x09,0x09,0x09,0x06},
      {0x3E,0x41,0x51,0x21,0x5E}, {0x7F,0x09,0x19,0x29,0x46},
      {0x46,0x49,0x49,0x49,0x31}, {0x01,0x01,0x7F,0x01,0x01},
      {0x3F,0x40,0x40,0x40,0x3F}, {0x1F,0x20,0x40,0x20,0x1F},
      {0x3F,0x40,0x38,0x40,0x3F}, {0x63,0x14,0x08,0x14,0x63},
      {0x07,0x08,0x70,0x08,0x07}, {0x61,0x51,0x49,0x45,0x43}
    };
    static const uint8_t blank[5] = {0,0,0,0,0};
    char c = input;
    if (c >= 'a' && c <= 'z') c -= 32;
    if (c >= '0' && c <= '9') return font[c - '0'];
    if (c >= 'A' && c <= 'Z') return font[10 + c - 'A'];
    return blank;
  }
};

} // namespace SUI
