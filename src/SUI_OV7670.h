#pragma once

#include <Arduino.h>
#include <Wire.h>

// Native OV7670 SCCB/register control. Frame capture is provided by a
// board-specific backend; this class deliberately does not pretend that
// arbitrary Arduino GPIO polling can capture a VGA pixel stream.
namespace SUI {

struct OV7670Pins {
  int8_t vcc = -1;   // Documentation only: never drive a power pin from GPIO.
  int8_t dgnd = -1;  // Documentation only: connect to GND.
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
  int8_t ret = -1;   // Kept as RET; its electrical function is board-specific.
  int8_t pwdn = -1;
};

class OV7670Camera {
public:
  explicit OV7670Camera(const OV7670Pins &pins, TwoWire &wire = Wire,
                        uint8_t address = 0x21)
      : _pins(pins), _wire(wire), _address(address) {}

  // Starts native SCCB communication and verifies the sensor product ID.
  // This does not start pixel-clock generation or frame capture.
  bool begin() {
    if (_pins.scl >= 0 && _pins.sda >= 0) {
#if defined(ARDUINO_ARCH_ESP32)
      _wire.begin((int)_pins.sda, (int)_pins.scl);
#else
      // Most Arduino cores use fixed hardware-I2C pins. The pin values remain
      // available in OV7670Pins, but Wire.begin() selects the board's I2C pins.
      _wire.begin();
#endif
    } else {
      _wire.begin();
    }

    if (_pins.pwdn >= 0) {
      pinMode((uint8_t)_pins.pwdn, OUTPUT);
      digitalWrite((uint8_t)_pins.pwdn, LOW);
      delay(1);
    }

    // Do not toggle RET automatically: its meaning is not confirmed for this
    // module's silkscreen/revision.
    delay(10);
    uint8_t productId = 0;
    if (!readRegister(0x0A, productId)) {
      _ready = false;
      return false;
    }
    _productId = productId;
    _ready = (productId == 0x76);
    return _ready;
  }

  void end() {
    _ready = false;
    _wire.end();
  }

  bool ready() const { return _ready; }
  uint8_t productId() const { return _productId; }
  const OV7670Pins &pins() const { return _pins; }

  bool writeRegister(uint8_t reg, uint8_t value) {
    _wire.beginTransmission(_address);
    _wire.write(reg);
    _wire.write(value);
    return _wire.endTransmission() == 0;
  }

  bool readRegister(uint8_t reg, uint8_t &value) {
    _wire.beginTransmission(_address);
    _wire.write(reg);
    if (_wire.endTransmission(false) != 0) return false;
    if (_wire.requestFrom((int)_address, 1) != 1) return false;
    value = (uint8_t)_wire.read();
    return true;
  }

  // Sensor software reset (COM7[7]). The sensor must already have XCLK.
  bool resetSensor() {
    if (!writeRegister(0x12, 0x80)) return false;
    delay(10);
    return true;
  }

  // Raw RGB/YUV mode and window/clock configuration should be explicit so
  // applications can select a mode appropriate to their capture backend.

private:
  OV7670Pins _pins;
  TwoWire &_wire;
  uint8_t _address;
  uint8_t _productId = 0;
  bool _ready = false;
};

} // namespace SUI
