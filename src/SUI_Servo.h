#pragma once

#include <Arduino.h>
#include <Servo.h>

namespace SUI {

class ServoMotor {
public:
  explicit ServoMotor(uint8_t pin) : _pin(pin), _attached(false) {}

  bool begin() {
    _attached = (_servo.attach(_pin) != INVALID_SERVO);
    return _attached;
  }

  void writeAngle(int angle) {
    if (!_attached) return;
    _servo.write(constrain(angle, 0, 180));
  }

  void detach() {
    _servo.detach();
    _attached = false;
  }

  bool attached() const { return _attached; }

private:
  uint8_t _pin;
  bool _attached;
  Servo _servo;
};

} // namespace SUI
