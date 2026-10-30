#pragma once

#include <Arduino.h>
#include <Wire.h>

namespace SUI {
enum class ActiveLevel : uint8_t { HIGH_LEVEL = HIGH, LOW = LOW };
class Relay {
public:
  explicit Relay(uint8_t pin,ActiveLevel active=ActiveLevel::HIGH_LEVEL):_pin(pin),_active(active),_on(false){}
  void begin(){pinMode(_pin,OUTPUT);off();}
  void on(){_on=true;digitalWrite(_pin,activeValue());}
  void off(){_on=false;digitalWrite(_pin,inactiveValue());}
  void toggle(){_on?off():on();}
  bool isOn()const{return _on;}
private:
  uint8_t _pin;ActiveLevel _active;bool _on;
  uint8_t activeValue()const{return _active==ActiveLevel::LOW?LOW:HIGH;}
  uint8_t inactiveValue()const{return _active==ActiveLevel::LOW?HIGH:LOW;}
};
class Button {
public:
  explicit Button(uint8_t pin,bool usePullup=true):_pin(pin),_pullup(usePullup){}
  void begin(){pinMode(_pin,_pullup?INPUT_PULLUP:INPUT);}
  bool isPressed()const{int v=digitalRead(_pin);return _pullup?v==LOW:v==HIGH;}
private:uint8_t _pin;bool _pullup;
};
class Buzzer {
public:
  explicit Buzzer(uint8_t pin):_pin(pin){}
  void begin(){pinMode(_pin,OUTPUT);noTone(_pin);}
  void beep(unsigned int frequency=1000,unsigned long durationMs=100){tone(_pin,frequency,durationMs);}
  void play(unsigned int frequency){tone(_pin,frequency);}
  void stop(){noTone(_pin);}
private:uint8_t _pin;
};
class HCSR04 {
public:
  HCSR04(uint8_t triggerPin,uint8_t echoPin,unsigned long timeoutUs=30000UL):_trigger(triggerPin),_echo(echoPin),_timeout(timeoutUs){}
  void begin(){pinMode(_trigger,OUTPUT);pinMode(_echo,INPUT);digitalWrite(_trigger,LOW);}
  float distanceCm(){digitalWrite(_trigger,LOW);delayMicroseconds(2);digitalWrite(_trigger,HIGH);delayMicroseconds(10);digitalWrite(_trigger,LOW);unsigned long d=pulseIn(_echo,HIGH,_timeout);return d==0?-1.0f:(d*0.0343f)/2.0f;}
private:uint8_t _trigger,_echo;unsigned long _timeout;
};
}
#include "SUI_Modules.h"
#include "SUI_Controllers.h"
#include "SUI_I2CLCD.h"
#include "SUI_ParallelLCD.h"

#if defined(SUI_ENABLE_SERVO)
#include "SUI_Servo.h"
#endif
#if defined(SUI_ENABLE_TFT)
#include "SUI_AdafruitTFT.h"
#endif
#if defined(SUI_ENABLE_OLED)
#include "SUI_AdafruitOLED.h"
#endif
#if defined(SUI_ENABLE_SD)
#include "SUI_SD.h"
#endif
#if defined(SUI_ENABLE_DFPLAYER)
#include "SUI_DFPlayer.h"
#endif
#if defined(SUI_ENABLE_OV7670)
#include "SUI_OV7670.h"
#endif
#if defined(SUI_ENABLE_DHT)
#include "SUI_DHT.h"
#endif
#if defined(SUI_ENABLE_DS18B20)
#include "SUI_DS18B20.h"
#endif
#if defined(SUI_ENABLE_BMP280)
#include "SUI_BMP280.h"
#endif
