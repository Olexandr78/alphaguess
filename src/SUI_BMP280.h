#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
namespace SUI {
class BMP280Sensor {
public:
  bool begin(uint8_t address=0x76){return _sensor.begin(address);}
  float temperatureC(){return _sensor.readTemperature();}
  float pressurePa(){return _sensor.readPressure();}
  float altitudeMeters(float seaLevelhPa=1013.25f){return _sensor.readAltitude(seaLevelhPa);}
  Adafruit_BMP280 &driver(){return _sensor;}
private:Adafruit_BMP280 _sensor;
};
}
