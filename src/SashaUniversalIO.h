#pragma once

#include <Arduino.h>
#include <Wire.h>

namespace SUI {

enum class ActiveLevel : uint8_t {
  HIGH_LEVEL = HIGH,
  LOW = LOW
};

class Relay {
public:
  explicit Relay(uint8_t pin, ActiveLevel active = ActiveLevel::HIGH_LEVEL)
    : _pin(pin), _active(active), _on(false) {}

  void begin() {
    pinMode(_pin, OUTPUT);
    off();
  }

  void on() {
    _on = true;
    digitalWrite(_pin, activeValue());
  }

  void off() {
    _on = false;
    digitalWrite(_pin, inactiveValue());
  }

  void toggle() { _on ? off() : on(); }
  bool isOn() const { return _on; }

private:
  uint8_t _pin;
  ActiveLevel _active;
  bool _on;

  uint8_t activeValue() const {
    return _active == ActiveLevel::LOW ? LOW : HIGH;
  }

  uint8_t inactiveValue() const {
    return _active == ActiveLevel::LOW ? HIGH : LOW;
  }
};

class Button {
public:
  explicit Button(uint8_t pin, bool usePullup = true)
    : _pin(pin), _pullup(usePullup) {}

  void begin() {
    pinMode(_pin, _pullup ? INPUT_PULLUP : INPUT);
  }

  // With INPUT_PULLUP, a pressed button connects the pin to GND.
  bool isPressed() const {
    const int value = digitalRead(_pin);
    return _pullup ? value == LOW : value == HIGH;
  }

private:
  uint8_t _pin;
  bool _pullup;
};

class Buzzer {
public:
  explicit Buzzer(uint8_t pin) : _pin(pin) {}

  void begin() {
    pinMode(_pin, OUTPUT);
    noTone(_pin);
  }

  void beep(unsigned int frequency = 1000, unsigned long durationMs = 100) {
    tone(_pin, frequency, durationMs);
  }

  void play(unsigned int frequency) { tone(_pin, frequency); }
  void stop() { noTone(_pin); }

private:
  uint8_t _pin;
};

class HCSR04 {
public:
  HCSR04(uint8_t triggerPin, uint8_t echoPin, unsigned long timeoutUs = 30000UL)
    : _trigger(triggerPin), _echo(echoPin), _timeout(timeoutUs) {}

  void begin() {
    pinMode(_trigger, OUTPUT);
    pinMode(_echo, INPUT);
    digitalWrite(_trigger, LOW);
  }

  // Returns -1.0 when no echo is received before the timeout.
  float distanceCm() {
    digitalWrite(_trigger, LOW);
    delayMicroseconds(2);
    digitalWrite(_trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trigger, LOW);

    const unsigned long duration = pulseIn(_echo, HIGH, _timeout);
    if (duration == 0) return -1.0f;
    return (duration * 0.0343f) / 2.0f;
  }

private:
  uint8_t _trigger;
  uint8_t _echo;
  unsigned long _timeout;
};

} // namespace SUI
