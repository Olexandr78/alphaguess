#define SUI_ENABLE_OV7670
#include <SashaUniversalIO.h>

// ESP32-WROOM-32 DevKit + OV7670 (no FIFO).
// Constructor order: SCL, SDA, VS, HS, PLK/PCLK, XLK/XCLK, D7..D0, RET, PWDN.
// This is a configurable example: edit these GPIOs to match your wiring.
// GPIO 34..39 are input-only and suitable for camera input signals.
// XCLK must use an output-capable GPIO. Avoid GPIO 6..11 (flash).
// Connect camera 3.3V to 3V3 and DGND to GND. RET is intentionally unused.

SUI::OV7670Camera camera(
  21, 22,              // SCL, SDA
  25, 23, 19, 18,      // VS, HS, PCLK, XCLK
  32, 33, 34, 35,      // D7, D6, D5, D4
  36, 39, 14, 27,      // D3, D2, D1, D0
  -1, 26               // RET, PWDN
);

void setup() {
  Serial.begin(115200);
  delay(300);

  // Optional: choose another frequency before begin().
  // camera.setXclkFrequency(8000000UL);

  if (!camera.begin()) {
    Serial.println("OV7670 init failed.");
    Serial.println("Check 3.3V/GND, XCLK output, SCCB wiring and module pinout.");
    Serial.print("XCLK running: ");
    Serial.println(camera.xclkRunning() ? "yes" : "no");
    return;
  }

  Serial.print("OV7670 PID: 0x");
  Serial.println(camera.productId(), HEX);
  Serial.print("XCLK: ");
  Serial.print(camera.xclkFrequency());
  Serial.println(" Hz");

  if (!camera.resetSensor()) {
    Serial.println("Sensor reset failed.");
    return;
  }

  Serial.println("SCCB control initialized; XCLK is active.");
  Serial.println("Frame capture is NOT implemented in this driver yet.");
}

void loop() {}
