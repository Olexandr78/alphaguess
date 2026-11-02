#define SUI_ENABLE_OV7670
#include <SashaUniversalIO.h>

// ESP32-WROOM-32 DevKit + OV7670 without FIFO.
// Edit these GPIOs to match your own wiring.
// GPIO 34..39 are input-only; XCLK must be output-capable.
// Camera 3.3V -> ESP32 3V3; DGND -> GND. RET is left disconnected.

SUI::OV7670Camera camera(
  21, 22,              // SCCB SCL, SDA
  25, 23, 19, 18,      // VS, HS/HREF, PCLK(PLK), XCLK(XLK)
  32, 33, 34, 35,      // D7, D6, D5, D4
  36, 39, 14, 27,      // D3, D2, D1, D0
  -1, 26               // RET, PWDN
);

static uint8_t frame[SUI::OV7670Camera::FRAME_BYTES];

void setup() {
  Serial.begin(115200);
  delay(300);

  if (!camera.begin()) {
    Serial.println("OV7670 init failed. Check power, XCLK, SCCB and pin mapping.");
    Serial.print("XCLK running: ");
    Serial.println(camera.xclkRunning() ? "yes" : "no");
    return;
  }

  Serial.print("OV7670 PID: 0x");
  Serial.println(camera.productId(), HEX);
  Serial.print("Mode: ");
  Serial.print(SUI::OV7670Camera::FRAME_WIDTH);
  Serial.print("x");
  Serial.print(SUI::OV7670Camera::FRAME_HEIGHT);
  Serial.println(" RGB565");

  Serial.println("Starting I2S0 DMA frame capture...");
}

void loop() {
  if (!camera.ready()) {
    delay(1000);
    return;
  }

  const size_t received = camera.captureFrame(frame, sizeof(frame), 2000);
  if (received == 0) {
    Serial.println("Capture timeout/error. Check VS, HS, PCLK and D0-D7 wiring.");
    delay(1000);
    return;
  }

  uint32_t checksum = 0;
  for (size_t i = 0; i < received; ++i) checksum = (checksum * 33u) ^ frame[i];

  Serial.print("Frame received: ");
  Serial.print(received);
  Serial.print(" bytes; checksum=0x");
  Serial.println(checksum, HEX);
  Serial.print("First 16 bytes: ");
  for (size_t i = 0; i < 16; ++i) {
    if (frame[i] < 16) Serial.print('0');
    Serial.print(frame[i], HEX);
    Serial.print(i == 15 ? '\\n' : ' ');
  }
  delay(500);
}
