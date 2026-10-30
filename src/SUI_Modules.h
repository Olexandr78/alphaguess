#pragma once
#include <Arduino.h>
#include <Stream.h>
#include <limits.h>

namespace SUI {

class DigitalInput {
public:
  explicit DigitalInput(uint8_t pin,bool activeLow=false,bool pullup=false)
    :_pin(pin),_activeLow(activeLow),_pullup(pullup){}
  void begin(){pinMode(_pin,_pullup?INPUT_PULLUP:INPUT);}
  bool read()const{int v=digitalRead(_pin);return _activeLow?v==LOW:v==HIGH;}
  int raw()const{return digitalRead(_pin);}
private:uint8_t _pin;bool _activeLow,_pullup;
};

class AnalogInput {
public:
  explicit AnalogInput(uint8_t pin):_pin(pin){}
  void begin(){pinMode(_pin,INPUT);}
  int read()const{return analogRead(_pin);}
private:uint8_t _pin;
};

class PWMOutput {
public:
  explicit PWMOutput(uint8_t pin):_pin(pin){}
  void begin(){pinMode(_pin,OUTPUT);write(0);}
  void write(uint8_t value){analogWrite(_pin,value);}
  void off(){write(0);}
private:uint8_t _pin;
};

class DebouncedButton {
public:
  explicit DebouncedButton(uint8_t pin,unsigned long debounceMs=30)
    :_pin(pin),_debounce(debounceMs),_lastRaw(HIGH),_stable(HIGH),_changedAt(0),_pressedEvent(false),_releasedEvent(false){}
  void begin(){pinMode(_pin,INPUT_PULLUP);_lastRaw=_stable=digitalRead(_pin);_changedAt=millis();}
  void update(){
    _pressedEvent=false;_releasedEvent=false;
    bool raw=digitalRead(_pin);
    if(raw!=_lastRaw){_lastRaw=raw;_changedAt=millis();}
    if((millis()-_changedAt)>=_debounce && raw!=_stable){
      _stable=raw;_pressedEvent=(_stable==LOW);_releasedEvent=(_stable==HIGH);
    }
  }
  bool isPressed()const{return _stable==LOW;}
  bool wasPressed(){bool v=_pressedEvent;_pressedEvent=false;return v;}
  bool wasReleased(){bool v=_releasedEvent;_releasedEvent=false;return v;}
private:uint8_t _pin;unsigned long _debounce;bool _lastRaw,_stable;unsigned long _changedAt;bool _pressedEvent,_releasedEvent;
};

class Joystick {
public:
  Joystick(uint8_t xPin,uint8_t yPin,uint8_t switchPin):_x(xPin),_y(yPin),_sw(switchPin){}
  void begin(){pinMode(_sw,INPUT_PULLUP);}
  int x()const{return analogRead(_x);}
  int y()const{return analogRead(_y);}
  bool pressed()const{return digitalRead(_sw)==LOW;}
  void read(int &xValue,int &yValue,bool &buttonPressed)const{xValue=x();yValue=y();buttonPressed=pressed();}
private:uint8_t _x,_y,_sw;
};

class RotaryEncoder {
public:
  RotaryEncoder(uint8_t clkPin,uint8_t dtPin,int8_t buttonPin=-1)
    :_clk(clkPin),_dt(dtPin),_sw(buttonPin),_lastState(0),_position(0){}
  void begin(){
    pinMode(_clk,INPUT_PULLUP);pinMode(_dt,INPUT_PULLUP);
    if(_sw>=0)pinMode((uint8_t)_sw,INPUT_PULLUP);
    _lastState=(uint8_t)((digitalRead(_clk)<<1)|digitalRead(_dt));
  }
  // Quadrature transition table rejects many invalid/bounce transitions.
  int8_t update(){
    static const int8_t table[16]={0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
    uint8_t state=(uint8_t)((digitalRead(_clk)<<1)|digitalRead(_dt));
    int8_t delta=table[(_lastState<<2)|state];
    _position+=delta;_lastState=state;return delta;
  }
  long position()const{return _position;}
  void setPosition(long value){_position=value;}
  bool pressed()const{return _sw>=0&&digitalRead((uint8_t)_sw)==LOW;}
private:uint8_t _clk,_dt,_lastState;int8_t _sw;long _position;
};

class Stepper4Wire {
public:
  Stepper4Wire(uint8_t p1,uint8_t p2,uint8_t p3,uint8_t p4,unsigned int stepDelayMs=3)
    :_pins{p1,p2,p3,p4},_delay(stepDelayMs),_phase(0){}
  void begin(){for(uint8_t i=0;i<4;i++){pinMode(_pins[i],OUTPUT);digitalWrite(_pins[i],LOW);}}
  void setStepDelay(unsigned int ms){_delay=ms;}
  void step(long count){
    const int8_t dir=count>=0?1:-1;
    while(count){_phase=(uint8_t)((_phase+(dir>0?1:7))&7);applyPhase();delay(_delay);count-=dir;}
  }
  void release(){for(uint8_t i=0;i<4;i++)digitalWrite(_pins[i],LOW);}
private:
  uint8_t _pins[4],_phase;unsigned int _delay;
  void applyPhase(){
    static const uint8_t seq[8][4]={{1,0,0,0},{1,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,1,0},{0,0,1,1},{0,0,0,1},{1,0,0,1}};
    for(uint8_t i=0;i<4;i++)digitalWrite(_pins[i],seq[_phase][i]?HIGH:LOW);
  }
};

class SevenSegment {
public:
  SevenSegment(const uint8_t segmentPins[8],bool commonAnode=false):_commonAnode(commonAnode){for(uint8_t i=0;i<8;i++)_pins[i]=segmentPins[i];}
  void begin(){for(uint8_t i=0;i<8;i++)pinMode(_pins[i],OUTPUT);clear();}
  void clear(){for(uint8_t i=0;i<8;i++)writeSegment(i,false);}
  void digit(uint8_t value,bool decimalPoint=false){
    static const uint8_t map[10]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};
    if(value>9){clear();return;}
    for(uint8_t i=0;i<7;i++)writeSegment(i,(map[value]>>i)&1);
    writeSegment(7,decimalPoint);
  }
  void character(char c){if(c>='0'&&c<='9')digit((uint8_t)(c-'0'));else if(c=='-'){clear();writeSegment(6,true);}else clear();}
private:
  uint8_t _pins[8];bool _commonAnode;
  void writeSegment(uint8_t i,bool on){digitalWrite(_pins[i],(on!=_commonAnode)?HIGH:LOW);}
};

class DipSwitch {
public:
  DipSwitch(const uint8_t *pins,uint8_t count):_count(count>8?8:count){for(uint8_t i=0;i<_count;i++)_pins[i]=pins[i];}
  void begin(){for(uint8_t i=0;i<_count;i++)pinMode(_pins[i],INPUT_PULLUP);}
  bool read(uint8_t index)const{return index<_count&&digitalRead(_pins[index])==LOW;}
  uint8_t value()const{uint8_t result=0;for(uint8_t i=0;i<_count;i++)if(read(i))result|=(uint8_t)(1U<<i);return result;}
private:uint8_t _pins[8],_count;
};

class AnalogMicrophone {
public:
  explicit AnalogMicrophone(uint8_t analogPin):_pin(analogPin){}
  void begin(){pinMode(_pin,INPUT);}
  int sample()const{return analogRead(_pin);}
  int peakToPeak(unsigned long windowMs=50)const{
    const unsigned long start=millis();int low=INT_MAX,high=INT_MIN;
    while(millis()-start<windowMs){int v=analogRead(_pin);if(v<low)low=v;if(v>high)high=v;}
    return (low==INT_MAX)?0:high-low;
  }
private:uint8_t _pin;
};

class SoundDetector {
public:
  explicit SoundDetector(uint8_t pin,bool activeLow=true):_pin(pin),_activeLow(activeLow){}
  void begin(){pinMode(_pin,INPUT);}
  bool detected()const{int v=digitalRead(_pin);return _activeLow?v==LOW:v==HIGH;}
private:uint8_t _pin;bool _activeLow;
};

class ActiveBeeper {
public:
  explicit ActiveBeeper(uint8_t pin,bool activeHigh=true):_pin(pin),_activeHigh(activeHigh){}
  void begin(){pinMode(_pin,OUTPUT);off();}
  void on(){digitalWrite(_pin,_activeHigh?HIGH:LOW);}
  void off(){digitalWrite(_pin,_activeHigh?LOW:HIGH);}
  void beep(unsigned long ms=100){on();delay(ms);off();}
private:uint8_t _pin;bool _activeHigh;
};
using PassiveBeeper=Buzzer;

class BluetoothUART {
public:
  explicit BluetoothUART(Stream &serial):_serial(serial){}
  int available(){return _serial.available();}
  int read(){return _serial.read();}
  size_t write(uint8_t value){return _serial.write(value);}
  size_t print(const char *value){return _serial.print(value);}
  size_t println(const char *value){return _serial.println(value);}
  Stream &stream(){return _serial;}
private:Stream &_serial;
};

// HW-748 is a breakout/kit for SIM800L; this class exposes raw AT UART only.
class SIM800L {
public:
  explicit SIM800L(Stream &serial):_serial(serial){}
  void sendAT(const char *command){_serial.print(command);_serial.print("\r\n");}
  int available(){return _serial.available();}
  int read(){return _serial.read();}
  Stream &stream(){return _serial;}
private:Stream &_serial;
};
} // namespace SUI
