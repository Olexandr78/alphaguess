#pragma once
#include <Arduino.h>
#include <math.h>
#include <DHT.h>
namespace SUI {
class DHTSensor {
public:
  DHTSensor(uint8_t pin,uint8_t type):_sensor(pin,type){}
  void begin(){_sensor.begin();}
  float temperatureC(){return _sensor.readTemperature();}
  float temperatureF(){return _sensor.readTemperature(true);}
  float humidity(){return _sensor.readHumidity();}
  bool valid(float value)const{return !isnan(value);}
private:DHT _sensor;
};
}
