#pragma once
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
namespace SUI {
class DS18B20 {
public:
  explicit DS18B20(uint8_t dataPin):_wire(dataPin),_sensors(&_wire){}
  void begin(){_sensors.begin();}
  void request(){_sensors.requestTemperatures();}
  float temperatureC(uint8_t index=0){return _sensors.getTempCByIndex(index);}
  float temperatureF(uint8_t index=0){return _sensors.getTempFByIndex(index);}
  uint8_t deviceCount(){return _sensors.getDeviceCount();}
private:OneWire _wire;DallasTemperature _sensors;
};
}
