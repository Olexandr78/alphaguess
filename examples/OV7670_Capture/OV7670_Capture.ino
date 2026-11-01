#define SUI_ENABLE_OV7670
#include <SashaUniversalIO.h>

// Set these GPIO numbers to match YOUR wiring.
// ESP32-WROOM-32 GPIO matrix allows the camera input signals to use
// different suitable GPIOs; this is only an example mapping.
//
// Important ESP32-WROOM-32 restrictions:
// - GPIO 34..39 are input-only (fine for D0..D7, VS, HS and PCLK).
// - XCLK must use an output-capable GPIO.
// - Avoid GPIO 6..11 (connected to flash).
// - GPIO 0, 2, 5, 12 and 15 are strapping pins; use carefully.
// - 3.3V and DGND are power connections, not GPIOs.
// - RET is left unassigned because its function depends on the module.

SUI::OV7670Pins cameraPins;

void configureCameraPins() {
  cameraPins.scl  = 21; // SCL
  cameraPins.sda  = 22; // SDA

  cameraPins.vs   = 25; // VS
  cameraPins.hs   = 23; // HS
  cameraPins.pclk = 19; // PLK (PCLK)
  cameraPins.xclk = 18; // XLK (XCLK)

  cameraPins.d7 = 32; // D7
  cameraPins.d6 = 33; // D6
  cameraPins.d5 = 34; // D5
  cameraPins.d4 = 35; // D4
  cameraPins.d3 = 36; // D3
  cameraPins.d2 = 39; // D2
  cameraPins.d1 = 14; // D1
  cameraPins.d0 = 27; // D0

  cameraPins.pwdn = 26; // PWDN; set -1 if not connected
  cameraPins.ret  = -1; // RET; do not drive until its function is confirmed

  // Power pins are documentation-only. Never connect them to GPIO:
  cameraPins.vcc = -1;  // 3.3V
  cameraPins.dgnd = -1; // DGND / GND
}

SUI::OV7670Camera camera(cameraPins);

void setup() {
  Serial.begin(115200);
  delay(500);

  configureCameraPins();

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
