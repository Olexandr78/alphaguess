#pragma once

#include <Arduino.h>
#include <Wire.h>

#if defined(ARDUINO_ARCH_ESP32)
  #include <esp_arduino_version.h>
#endif

// Native OV7670 SCCB control and ESP32 XCLK generation.
// This class does NOT claim to capture pixels: the ESP32-WROOM-32 needs a
// separate I2S/DMA parallel-input backend for that.
namespace SUI {

struct OV7670Pins {
  int8_t vcc = -1;   // Documentation only. Connect to a suitable 3.3V supply.
  int8_t dgnd = -1;  // Documentation only. Connect to GND.
  int8_t scl = -1;
  int8_t sda = -1;
  int8_t vs = -1;
  int8_t hs = -1;
  int8_t pclk = -1;  // Module silkscreen: PLK
  int8_t xclk = -1;  // Module silkscreen: XLK
  int8_t d7 = -1;
  int8_t d6 = -1;
  int8_t d5 = -1;
  int8_t d4 = -1;
  int8_t d3 = -1;
  int8_t d2 = -1;
  int8_t d1 = -1;
  int8_t d0 = -1;
  int8_t ret = -1;   // Preserve the exact board label; function is unconfirmed.
  int8_t pwdn = -1;
};

class OV7670Camera {
public:
  // Constructor order: SCL, SDA, VS, HS, PCLK(PLK), XCLK(XLK),
  // D7..D0, RET, PWDN. RET and PWDN may be -1 when not connected.
  OV7670Camera(int8_t scl, int8_t sda, int8_t vs, int8_t hs,
               int8_t pclk, int8_t xclk,
               int8_t d7, int8_t d6, int8_t d5, int8_t d4,
               int8_t d3, int8_t d2, int8_t d1, int8_t d0,
               int8_t ret = -1, int8_t pwdn = -1,
               TwoWire &wire = Wire, uint8_t address = 0x21)
      : _pins(), _wire(wire), _address(address) {
    _pins.scl = scl; _pins.sda = sda;
    _pins.vs = vs; _pins.hs = hs;
    _pins.pclk = pclk; _pins.xclk = xclk;
    _pins.d7 = d7; _pins.d6 = d6; _pins.d5 = d5; _pins.d4 = d4;
    _pins.d3 = d3; _pins.d2 = d2; _pins.d1 = d1; _pins.d0 = d0;
    _pins.ret = ret; _pins.pwdn = pwdn;
  }

  explicit OV7670Camera(const OV7670Pins &pins, TwoWire &wire = Wire,
                        uint8_t address = 0x21)
      : _pins(pins), _wire(wire), _address(address) {}

  // Set before begin(). OV7670 accepts a clock in the MHz range; 10 MHz is a
  // conservative starting point for this board. The actual output is hardware
  // LEDC PWM, not software pin toggling.
  bool setXclkFrequency(uint32_t hz) {
    if (_ready || _xclkStarted || hz < 1000000UL || hz > 24000000UL) return false;
    _xclkHz = hz;
    return true;
  }

  uint32_t xclkFrequency() const { return _xclkHz; }
  bool xclkRunning() const { return _xclkStarted; }
  bool ready() const { return _ready; }
  uint8_t productId() const { return _productId; }
  const OV7670Pins &pins() const { return _pins; }

  // Start XCLK BEFORE SCCB access. On ESP32, a connected XCLK GPIO is required.
  // On other Arduino architectures, XCLK must be supplied externally.
  bool begin() {
    _ready = false;

    if (_pins.pwdn >= 0) {
      pinMode((uint8_t)_pins.pwdn, OUTPUT);
      digitalWrite((uint8_t)_pins.pwdn, LOW);
      delay(2);
    }

#if defined(ARDUINO_ARCH_ESP32)
    if (_pins.xclk < 0 || !startXclk()) return false;
    delay(10); // Allow the sensor clock domain to settle before SCCB.
#endif

    bool wireOk;
    if (_pins.scl >= 0 && _pins.sda >= 0) {
#if defined(ARDUINO_ARCH_ESP32)
      wireOk = _wire.begin((int)_pins.sda, (int)_pins.scl);
#else
      wireOk = _wire.begin();
#endif
    } else {
      wireOk = _wire.begin();
    }
    if (!wireOk) return false;

    delay(2);
    uint8_t pid = 0;
    if (!readRegister(0x0A, pid)) return false;
    _productId = pid;
    _ready = (pid == 0x76);
    return _ready;
  }

  // Stop only resources owned by this camera. Wire may be shared by other
  // devices, so end() deliberately does not call TwoWire::end().
  void end() {
    _ready = false;
    stopXclk();
  }

  bool startXclk() {
#if defined(ARDUINO_ARCH_ESP32)
    if (_xclkStarted) return true;
    if (_pins.xclk < 0) return false;
    const uint8_t pin = (uint8_t)_pins.xclk;
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    if (!ledcAttach(pin, _xclkHz, 1)) return false;
    if (!ledcWrite(pin, 1)) {
      ledcDetach(pin);
      return false;
    }
#else
    if (ledcSetup(_xclkChannel, _xclkHz, 1) <= 0.0) return false;
    ledcAttachPin(pin, _xclkChannel);
    ledcWrite(_xclkChannel, 1);
#endif
    _xclkStarted = true;
    return true;
#else
    return false;
#endif
  }

  void stopXclk() {
#if defined(ARDUINO_ARCH_ESP32)
    if (!_xclkStarted) return;
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcDetach((uint8_t)_pins.xclk);
#else
    ledcWrite(_xclkChannel, 0);
    ledcDetachPin((uint8_t)_pins.xclk);
#endif
    _xclkStarted = false;
#endif
  }

  bool writeRegister(uint8_t reg, uint8_t value) {
    _wire.beginTransmission(_address);
    if (_wire.write(reg) != 1 || _wire.write(value) != 1) return false;
    return _wire.endTransmission() == 0;
  }

  bool readRegister(uint8_t reg, uint8_t &value) {
    _wire.beginTransmission(_address);
    if (_wire.write(reg) != 1) return false;
    if (_wire.endTransmission(false) != 0) return false;
    if (_wire.requestFrom((int)_address, 1) != 1) return false;
    const int received = _wire.read();
    if (received < 0) return false;
    value = (uint8_t)received;
    return true;
  }

  bool resetSensor() {
    if (!_ready || !writeRegister(0x12, 0x80)) return false;
    delay(10);
    return true;
  }

private:
  OV7670Pins _pins;
  TwoWire &_wire;
  uint8_t _address;
  uint8_t _productId = 0;
  uint32_t _xclkHz = 10000000UL;
  bool _ready = false;
  bool _xclkStarted = false;
#if defined(ARDUINO_ARCH_ESP32) && (!defined(ESP_ARDUINO_VERSION_MAJOR) || ESP_ARDUINO_VERSION_MAJOR < 3)
  // Dedicated legacy LEDC channel; change if the sketch already reserves it.
  uint8_t _xclkChannel = 7;
#endif
};

} // namespace SUI
