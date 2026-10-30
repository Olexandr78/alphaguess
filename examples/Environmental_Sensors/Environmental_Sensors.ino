#define SUI_ENABLE_DHT
#define SUI_ENABLE_DS18B20
#define SUI_ENABLE_BMP280
#include <SashaUniversalIO.h>

SUI::DHTSensor dht(2, DHT22);
SUI::DS18B20 probe(3);
SUI::BMP280Sensor pressure;
void setup() {
  Serial.begin(9600);
  dht.begin(); probe.begin();
  if (!pressure.begin(0x76)) Serial.println("BMP280 not found");
}
void loop() {
  probe.request();
  Serial.print("DHT C: "); Serial.println(dht.temperatureC());
  Serial.print("DHT RH: "); Serial.println(dht.humidity());
  Serial.print("DS18B20 C: "); Serial.println(probe.temperatureC());
  Serial.print("BMP280 hPa: "); Serial.println(pressure.pressurePa()/100.0f);
  delay(2000);
}
