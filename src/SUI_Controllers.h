#pragma once
#include <Arduino.h>

namespace SUI {

// STEP/DIR driver interface for A4988, DRV8825, TMC in STEP/DIR mode.
// Driver must have its own motor supply and current limit configured.
class StepDirStepper {
public:
  StepDirStepper(uint8_t stepPin, uint8_t dirPin, int8_t enablePin = -1,
                 bool enableActiveLow = true)
    : _step(stepPin), _dir(dirPin), _enable(enablePin),
      _enableLow(enableActiveLow), _pulseUs(3), _stepDelayUs(500) {}
  void begin() {
    pinMode(_step, OUTPUT); pinMode(_dir, OUTPUT); digitalWrite(_step, LOW);
    if (_enable >= 0) { pinMode((uint8_t)_enable, OUTPUT); disable(); }
  }
  void setTiming(uint16_t pulseWidthUs, uint16_t stepDelayUs) {
    _pulseUs = pulseWidthUs; _stepDelayUs = stepDelayUs;
  }
  void enable() {
    if (_enable >= 0) digitalWrite((uint8_t)_enable, _enableLow ? LOW : HIGH);
  }
  void disable() {
    if (_enable >= 0) digitalWrite((uint8_t)_enable, _enableLow ? HIGH : LOW);
  }
  void move(long steps) {
    if (!steps) return;
    digitalWrite(_dir, steps > 0 ? HIGH : LOW);
    const unsigned long count = steps > 0 ? (unsigned long)steps : (unsigned long)(-(steps + 1)) + 1UL;
    for (unsigned long i=0; i<count; ++i) {
      digitalWrite(_step, HIGH); delayMicroseconds(_pulseUs);
      digitalWrite(_step, LOW); delayMicroseconds(_stepDelayUs);
    }
  }
private:
  uint8_t _step, _dir;
  int8_t _enable;
  bool _enableLow;
  uint16_t _pulseUs, _stepDelayUs;
};

// TM1637 4-digit display, software clock/data implementation.
class TM1637Display {
public:
  TM1637Display(uint8_t clockPin, uint8_t dataPin)
    : _clk(clockPin), _dio(dataPin), _brightness(7) {}
  void begin() {
    pinMode(_clk, OUTPUT); pinMode(_dio, OUTPUT);
    digitalWrite(_clk, HIGH); digitalWrite(_dio, HIGH);
    clear();
  }
  void setBrightness(uint8_t value) { _brightness = value > 7 ? 7 : value; command((uint8_t)(0x88 | _brightness)); }
  void clear() { uint8_t blank[4]={0,0,0,0}; setSegments(blank); }
  void showNumber(int number, bool leadingZero=false) {
    static const uint8_t digits[10]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};
    uint8_t seg[4]={0,0,0,0};
    bool negative=number<0;
    unsigned long n=negative ? (unsigned long)(-(long)number) : (unsigned long)number;
    for(int i=3;i>=0;i--) {
      if(n || i==3 || leadingZero) { seg[i]=digits[n%10]; n/=10; }
      else if(negative && i==0) seg[i]=0x40;
    }
    if(negative) seg[0]=0x40;
    setSegments(seg);
  }
  void setSegments(const uint8_t data[4], uint8_t position=0, uint8_t length=4) {
    if(position>3) return;
    if(length>4-position) length=4-position;
    command(0x40);
    start();
    writeByte((uint8_t)(0xC0 | position));
    for(uint8_t i=0;i<length;i++) writeByte(data[i]);
    stop();
    command((uint8_t)(0x88 | _brightness));
  }
private:
  uint8_t _clk,_dio,_brightness;
  void start(){ digitalWrite(_dio,HIGH); digitalWrite(_clk,HIGH); digitalWrite(_dio,LOW); digitalWrite(_clk,LOW); }
  void stop(){ digitalWrite(_clk,LOW); digitalWrite(_dio,LOW); digitalWrite(_clk,HIGH); digitalWrite(_dio,HIGH); }
  bool writeByte(uint8_t value) {
    for(uint8_t i=0;i<8;i++){ digitalWrite(_clk,LOW); digitalWrite(_dio,(value&1)?HIGH:LOW); value>>=1; digitalWrite(_clk,HIGH); }
    digitalWrite(_clk,LOW); pinMode(_dio,INPUT); digitalWrite(_clk,HIGH);
    bool ack=digitalRead(_dio)==LOW; pinMode(_dio,OUTPUT); digitalWrite(_clk,LOW);
    return ack;
  }
  void command(uint8_t value){ start(); writeByte(value); stop(); }
};

// MAX7219/MAX7221 8-digit LED driver, hardware-independent bit-banged serial.
class MAX7219Display {
public:
  MAX7219Display(uint8_t dataPin,uint8_t clockPin,uint8_t csPin)
    : _din(dataPin),_clk(clockPin),_cs(csPin) {}
  void begin(uint8_t intensity=8) {
    pinMode(_din,OUTPUT); pinMode(_clk,OUTPUT); pinMode(_cs,OUTPUT);
    digitalWrite(_cs,HIGH); digitalWrite(_clk,LOW);
    writeRegister(0x0F,0); writeRegister(0x0C,1); writeRegister(0x0B,7);
    writeRegister(0x09,0); setIntensity(intensity); clear();
  }
  void setIntensity(uint8_t value){ writeRegister(0x0A,value>15?15:value); }
  void shutdown(bool value){ writeRegister(0x0C,value?0:1); }
  void clear(){ for(uint8_t i=1;i<=8;i++) writeRegister(i,0x0F); }
  void setRaw(uint8_t digit,uint8_t segments){ if(digit<8) writeRegister((uint8_t)(digit+1),segments); }
  void setDigit(uint8_t digit,uint8_t value,bool decimalPoint=false) {
    static const uint8_t map[10]={0x7e,0x30,0x6d,0x79,0x33,0x5b,0x5f,0x70,0x7f,0x7b};
    if(digit>=8) return;
    writeRegister((uint8_t)(digit+1),value<10?(uint8_t)(map[value]|(decimalPoint?0x80:0)):0x0F);
  }
private:
  uint8_t _din,_clk,_cs;
  void writeRegister(uint8_t address,uint8_t value) {
    digitalWrite(_cs,LOW);
    shiftOut(_din,_clk,MSBFIRST,address); shiftOut(_din,_clk,MSBFIRST,value);
    digitalWrite(_cs,HIGH);
  }
};

// 74HC595 output shift register, one byte per chip.
class ShiftRegister595 {
public:
  ShiftRegister595(uint8_t dataPin,uint8_t clockPin,uint8_t latchPin,bool msbFirst=true)
    : _data(dataPin),_clock(clockPin),_latch(latchPin),_msbFirst(msbFirst),_state(0) {}
  void begin(){ pinMode(_data,OUTPUT); pinMode(_clock,OUTPUT); pinMode(_latch,OUTPUT); write(0); }
  void write(uint8_t value){ _state=value; digitalWrite(_latch,LOW); shiftOut(_data,_clock,_msbFirst?MSBFIRST:LSBFIRST,_state); digitalWrite(_latch,HIGH); }
  void set(uint8_t bit,bool on){ if(bit>7)return; if(on)_state|=(uint8_t)(1U<<bit);else _state&=(uint8_t)~(1U<<bit); write(_state); }
  uint8_t value() const{return _state;}
private:
  uint8_t _data,_clock,_latch,_state;bool _msbFirst;
};

} // namespace SUI
