#pragma once

#include <Arduino.h>
#include <Wire.h>

// Minimal HD44780 character LCD driver for common PCF8574 backpacks.
// Default backpack mapping: P0=RS, P1=RW, P2=EN, P3=backlight,
// P4..P7=D4..D7. Some backpacks use a different mapping.
namespace SUI {

class I2CLCD {
public:
  I2CLCD(uint8_t address = 0x27, uint8_t columns = 16, uint8_t rows = 2)
    : _address(address), _columns(columns), _rows(rows), _backlight(true) {}

  void begin() {
    Wire.begin();
    delay(50);
    write4bits(0x03 << 4);
    delayMicroseconds(4500);
    write4bits(0x03 << 4);
    delayMicroseconds(4500);
    write4bits(0x03 << 4);
    delayMicroseconds(150);
    write4bits(0x02 << 4);
    command(0x28); // 4-bit, 2-line, 5x8 font
    command(0x0C); // display on, cursor off
    command(0x06); // entry mode
    clear();
  }

  void clear() {
    command(0x01);
    delay(2);
  }

  void home() {
    command(0x02);
    delay(2);
  }

  void setCursor(uint8_t column, uint8_t row) {
    static const uint8_t rowOffsets[] = {0x00, 0x40, 0x14, 0x54};
    if (row >= _rows) row = _rows - 1;
    if (column >= _columns) column = _columns - 1;
    command(0x80 | (column + rowOffsets[row]));
  }

  size_t write(uint8_t value) {
    send(value, true);
    return 1;
  }

  void print(const char *text) {
    if (!text) return;
    while (*text) write(static_cast<uint8_t>(*text++));
  }

  void backlight() {
    _backlight = true;
    expanderWrite(0);
  }

  void noBacklight() {
    _backlight = false;
    expanderWrite(0);
  }

private:
  uint8_t _address;
  uint8_t _columns;
  uint8_t _rows;
  bool _backlight;

  void command(uint8_t value) { send(value, false); }

  void send(uint8_t value, bool character) {
    const uint8_t mode = character ? 0x01 : 0x00;
    write4bits((value & 0xF0) | mode);
    write4bits(((value << 4) & 0xF0) | mode);
  }

  void write4bits(uint8_t value) {
    expanderWrite(value);
    pulseEnable(value);
  }

  void pulseEnable(uint8_t value) {
    expanderWrite(value | 0x04);
    delayMicroseconds(1);
    expanderWrite(value & static_cast<uint8_t>(~0x04));
    delayMicroseconds(50);
  }

  void expanderWrite(uint8_t value) {
    Wire.beginTransmission(_address);
    Wire.write(value | (_backlight ? 0x08 : 0x00));
    Wire.endTransmission();
  }
};

} // namespace SUI
