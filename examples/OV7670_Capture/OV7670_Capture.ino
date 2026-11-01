#define SUI_ENABLE_OV7670
#include <SashaUniversalIO.h>

// ESP32-WROOM-32 example only. Change the GPIO numbers below to match
// YOUR wiring. Constructor order:
// SCL, SDA, VS, HS, PLK(PCLK), XLK(XCLK), D7..D0, RET, PWDN.
//
// GPIO 34..39 are input-only and are suitable for camera input signals.
// XCLK needs an output-capable GPIO. Avoid GPIO 6..11 (flash).
// GPIO 0, 2, 5, 12 and 15 are strapping pins: use with care.
// Connect 3.3V to 3.3V and DGND to GND, never to GPIO.
// RET is left at -1 until its function on this module is confirmed.

SUI::OV7670Camera camera(
  21, 22,             // SCL, SDA
  25, 23, 19, 18,      // VS, HS, PLK/PCLK, XLK/XCLK
  32, 33, 34, 35,      // D7, D6, D5, D4
  36, 39, 14, 27,      // D3, D2, D1, D0
  -1, 26               // RET (unused), PWDN
);

void setup() {
  Serial.begin(115200);
  delay(500);

  if (!camera.begin()) {
    Serial.println("OV7670 not detected over SCCB; check power, XCLK, SCL and SDA.");
    while (true) delay(100);
  }

  Serial.print("OV7670 detected, PID=0x");
  Serial.println(camera.productId(), HEX);

  if (!camera.resetSensor()) {
    Serial.println("Sensor reset command failed.");
    while (true) delay(100);
  }

  Serial.println("OV7670 SCCB control is ready.");
  Serial.println("This example does not yet capture image frames.");
}

void loop() {}
