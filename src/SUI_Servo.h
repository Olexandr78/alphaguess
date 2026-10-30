#pragma once
#include <Arduino.h>
#if defined(ARDUINO_ARCH_ESP32)
#include <ESP32Servo.h>
#else
#include <Servo.h>
#endif

namespace SUI {
class ServoMotor {
public:
  explicit ServoMotor(uint8_t pin) : _pin(pin), _attached(false) {}
  bool begin() {
    _servo.attach(_pin);
    _attached = _servo.attached();
    return _attached;
  }
  void writeAngle(int angle) { if(_attached) _servo.write(constrain(angle,0,180)); }
  void writeMicroseconds(int value) { if(_attached) _servo.writeMicroseconds(constrain(value,500,2500)); }
  void detach(){ _servo.detach(); _attached=false; }
  bool attached() const { return _attached; }
private:
  uint8_t _pin;
  bool _attached;
  Servo _servo;
};
}
