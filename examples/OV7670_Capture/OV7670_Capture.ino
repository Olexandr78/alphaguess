#define SUI_ENABLE_OV7670
#include <SashaUniversalIO.h>

// This example needs a board supported by Arduino_OV767X (for example Nano 33 BLE).
// QCIF RGB565 = 176 * 144 * 2 bytes; this buffer is too large for an Uno/Nano AVR.
SUI::OV7670Camera camera;
uint8_t frame[176UL * 144UL * 2UL];

void setup() {
  Serial.begin(115200);
  if (!camera.begin(QCIF, RGB565, 1)) {
    Serial.println("OV7670 init failed or board unsupported");
    while (true) {}
  }
  Serial.print("Frame bytes: "); Serial.println(camera.frameSize());
}

void loop() {
  if (camera.capture(frame)) Serial.write(frame, camera.frameSize());
}
