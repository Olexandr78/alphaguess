#define SUI_ENABLE_OV7670
#include <SashaUniversalIO.h>

// ESP32-WROOM-32 wiring. Connect every camera signal to the GPIO listed here.
// D1 and D0 are assigned to GPIO14 and GPIO27 (they were previously unassigned).
// 3.3V and DGND are power connections, not GPIOs.
SUI::OV7670Pins cameraPins = {
  -1, // 3.3V
  -1, // DGND
  21, // SCL
  22, // SDA
  25, // VS
  23, // HS
  19, // PLK / PCLK
  18, // XLK / XCLK (clock output backend not implemented here)
  32, // D7
  33, // D6
  34, // D5
  35, // D4
  36, // D3
  39, // D2
  14, // D1
  27, // D0
  -1, // RET: intentionally not driven until its function is confirmed
  26  // PWDN
};

SUI::OV7670Camera camera(cameraPins);

void setup() {
  Serial.begin(115200);
  delay(500);

  if (!camera.begin()) {
    Serial.println("OV7670 not detected over SCCB; check power, SCL and SDA.");
    while (true) delay(100);
  }

  Serial.print("OV7670 detected, PID=0x");
  Serial.println(camera.productId(), HEX);

  if (!camera.resetSensor()) {
    Serial.println("Sensor reset command failed.");
    while (true) delay(100);
  }

  Serial.println("Native SCCB register control is ready.");
  Serial.println("Frame capture still needs an MCU-specific DMA backend.");
}

void loop() {}
