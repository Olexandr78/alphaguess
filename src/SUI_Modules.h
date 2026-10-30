#pragma once
#include <Arduino.h>
#include <Stream.h>

namespace SUI {

// HW-504 / KY-023 two-axis joystick.
class Joystick {
public:
  Joystick(uint8_t xPin, uint8_t yPin, uint8_t switchPin)
    : _x(xPin), _y(yPin), _sw(switchPin) {}
  void begin() { pinMode(_sw, INPUT_PULLUP); }
  int x() const { return analogRead(_x); }
  int y() const { return analogRead(_y); }
  bool pressed() const { return digitalRead(_sw) == LOW; }
  void read(int &xValue, int &yValue, bool &buttonPressed) const {
    xValue = x(); yValue = y(); buttonPressed = pressed();
  }
private:
  uint8_t _x, _y, _sw;
};

// HW-040 / KY-040 mechanical rotary encoder.
class RotaryEncoder {
public:
  RotaryEncoder(uint8_t clkPin, uint8_t dtPin, int8_t buttonPin = -1)
    : _clk(clkPin), _dt(dtPin), _sw(buttonPin), _last(0), _position(0) {}
  void begin() {
    pinMode(_clk, INPUT_PULLUP); pinMode(_dt, INPUT_PULLUP);
    if (_sw >= 0) pinMode(static_cast<uint8_t>(_sw), INPUT_PULLUP);
    _last = digitalRead(_clk);
  }
  // Call frequently from loop(); returns -1, 0 or +1 for this update.
  int8_t update() {
    int8_t change = 0;
    const uint8_t now = digitalRead(_clk);
    if (now != _last && now == LOW) {
      change = (digitalRead(_dt) != now) ? 1 : -1;
      _position += change;
    }
    _last = now;
    return change;
  }
  long position() const { return _position; }
  void setPosition(long value) { _position = value; }
  bool pressed() const { return _sw >= 0 && digitalRead(static_cast<uint8_t>(_sw)) == LOW; }
private:
  uint8_t _clk, _dt;
  int8_t _sw;
  uint8_t _last;
  long _position;
};

// Basic four-wire unipolar/bipolar stepper driver (ULN2003, L298N, etc.).
// This is a simple blocking full-step sequence, not a speed-controlled driver.
class Stepper4Wire {
public:
  Stepper4Wire(uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4,
               unsigned int stepDelayMs = 3)
    : _pins{p1,p2,p3,p4}, _delay(stepDelayMs) {}
  void begin() { for (uint8_t i=0;i<4;i++) { pinMode(_pins[i], OUTPUT); digitalWrite(_pins[i], LOW); } }
  void setStepDelay(unsigned int ms) { _delay = ms; }
  void step(long count) {
    const int8_t direction = count >= 0 ? 1 : -1;
    while (count != 0) {
      _phase = static_cast<uint8_t>((_phase + (direction > 0 ? 1 : 7)) & 7);
      applyPhase();
      delay(_delay);
      count -= direction;
    }
  }
  void release() { for (uint8_t i=0;i<4;i++) digitalWrite(_pins[i], LOW); }
private:
  uint8_t _pins[4], _phase = 0;
  unsigned int _delay;
  void applyPhase() {
    static const uint8_t seq[8][4] = {
      {1,0,0,0},{1,1,0,0},{0,1,0,0},{0,1,1,0},
      {0,0,1,0},{0,0,1,1},{0,0,0,1},{1,0,0,1}
    };
    for (uint8_t i=0;i<4;i++) digitalWrite(_pins[i], seq[_phase][i] ? HIGH : LOW);
  }
};

// Single-digit seven-segment display. Segment order: a,b,c,d,e,f,g,dp.
// Supply current-limiting resistors for every segment.
class SevenSegment {
public:
  SevenSegment(const uint8_t segmentPins[8], bool commonAnode = false)
    : _commonAnode(commonAnode) {
    for (uint8_t i=0;i<8;i++) _pins[i] = segmentPins[i];
  }
  void begin() { for (uint8_t i=0;i<8;i++) pinMode(_pins[i], OUTPUT); clear(); }
  void clear() { for (uint8_t i=0;i<8;i++) writeSegment(i, false); }
  void digit(uint8_t value, bool decimalPoint = false) {
    static const uint8_t map[10] = {
      0b00111111,0b00000110,0b01011011,0b01001111,0b01100110,
      0b01101101,0b01111101,0b00000111,0b01111111,0b01101111
    };
    if (value > 9) { clear(); return; }
    for (uint8_t i=0;i<7;i++) writeSegment(i, (map[value] >> i) & 1);
    writeSegment(7, decimalPoint);
  }
  void character(char c) {
    if (c >= '0' && c <= '9') digit(static_cast<uint8_t>(c-'0'));
    else if (c == '-') { clear(); writeSegment(6,true); }
    else clear();
  }
private:
  uint8_t _pins[8];
  bool _commonAnode;
  void writeSegment(uint8_t i, bool on) {
    digitalWrite(_pins[i], (on != _commonAnode) ? HIGH : LOW);
  }
};

// DIP switch bank: each switch is read as a digital input with pull-up.
class DipSwitch {
public:
  DipSwitch(const uint8_t *pins, uint8_t count) : _count(count > 8 ? 8 : count) {
    for (uint8_t i=0;i<_count;i++) _pins[i] = pins[i];
  }
  void begin() { for (uint8_t i=0;i<_count;i++) pinMode(_pins[i], INPUT_PULLUP); }
  bool read(uint8_t index) const {
    return index < _count && digitalRead(_pins[index]) == LOW;
  }
  uint8_t value() const {
    uint8_t result=0;
    for (uint8_t i=0;i<_count;i++) if (read(i)) result |= static_cast<uint8_t>(1U << i);
    return result;
  }
private:
  uint8_t _pins[8], _count;
};

// Analog microphone / sound sensor with analog output (AO).
// Electret capsules require a bias/preamp circuit; this reads an amplified module.
class AnalogMicrophone {
public:
  explicit AnalogMicrophone(uint8_t analogPin) : _pin(analogPin) {}
  void begin() { pinMode(_pin, INPUT); }
  int sample() const { return analogRead(_pin); }
  int peakToPeak(unsigned long windowMs = 50) const {
    const unsigned long start = millis();
    int low = 1023, high = 0;
    while (millis() - start < windowMs) {
      const int v = analogRead(_pin);
      if (v < low) low = v;
      if (v > high) high = v;
    }
    return high - low;
  }
private:
  uint8_t _pin;
};

// Digital sound detector output (DO), such as LM393 microphone modules.
class SoundDetector {
public:
  explicit SoundDetector(uint8_t pin, bool activeLow = true)
    : _pin(pin), _activeLow(activeLow) {}
  void begin() { pinMode(_pin, INPUT); }
  bool detected() const {
    const int v = digitalRead(_pin);
    return _activeLow ? v == LOW : v == HIGH;
  }
private:
  uint8_t _pin; bool _activeLow;
};

// Active buzzer is simply switched on/off; passive buzzer uses tone().
class ActiveBeeper {
public:
  explicit ActiveBeeper(uint8_t pin, bool activeHigh = true)
    : _pin(pin), _activeHigh(activeHigh) {}
  void begin() { pinMode(_pin, OUTPUT); off(); }
  void on() { digitalWrite(_pin, _activeHigh ? HIGH : LOW); }
  void off() { digitalWrite(_pin, _activeHigh ? LOW : HIGH); }
  void beep(unsigned long ms = 100) { on(); delay(ms); off(); }
private:
  uint8_t _pin; bool _activeHigh;
};
using PassiveBeeper = Buzzer;

// HC-05 / HC-06 Bluetooth UART helper. Pass Serial, Serial1, or SoftwareSerial.
class BluetoothUART {
public:
  explicit BluetoothUART(Stream &serial) : _serial(serial) {}
  int available() { return _serial.available(); }
  int read() { return _serial.read(); }
  size_t write(uint8_t value) { return _serial.write(value); }
  size_t print(const char *value) { return _serial.print(value); }
  size_t println(const char *value) { return _serial.println(value); }
  Stream &stream() { return _serial; }
private:
  Stream &_serial;
};

// HW-748 is commonly sold as a SIM800L GSM/GPRS module.
// This low-level AT helper does not implement a complete GSM stack.
class SIM800L {
public:
  explicit SIM800L(Stream &serial) : _serial(serial) {}
  void sendAT(const char *command) {
    _serial.print(command);
    _serial.print("\r\n");
  }
  int available() { return _serial.available(); }
  int read() { return _serial.read(); }
  Stream &stream() { return _serial; }
private:
  Stream &_serial;
};

} // namespace SUI
