import hashlib
import os

FILES = {
    "platformio.ini": """[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 921600
lib_extra_dirs = lib/SashaUniversalIO
""",
    "lib/SashaUniversalIO/library.properties": """name=SashaUniversalIO
version=0.5.0
author=Olexandr78
maintainer=Olexandr78
sentence=Unified Arduino helpers for displays, sensors, motors, audio and communication modules.
paragraph=Single umbrella include with built-in drivers and optional adapters for common sensor and peripheral families.
category=Device Control
url=https://github.com/Olexandr78/alphaguess
architectures=*
includes=SashaUniversalIO.h
""",
    "lib/SashaUniversalIO/src/SashaUniversalIO.h": """#pragma once

#include <Arduino.h>
#include <Wire.h>

#if defined(SUI_ENABLE_OV7670)
#include "SUI_OV7670.h"
#endif
""",
    "lib/SashaUniversalIO/src/SUI_OV7670.h": """#pragma once

#include <Arduino.h>
#include <Wire.h>

namespace SUI {

struct OV7670Pins {
  int8_t scl = -1;
  int8_t sda = -1;
  int8_t vs = -1;
  int8_t hs = -1;
  int8_t pclk = -1;
  int8_t xclk = -1;
  int8_t d7 = -1;
  int8_t d6 = -1;
  int8_t d5 = -1;
  int8_t d4 = -1;
  int8_t d3 = -1;
  int8_t d2 = -1;
  int8_t d1 = -1;
  int8_t d0 = -1;
  int8_t ret = -1;
  int8_t pwdn = -1;
};

class OV7670Camera {
public:
  explicit OV7670Camera(const OV7670Pins &pins, TwoWire &wire = Wire, uint8_t address = 0x21)
      : _pins(pins), _wire(wire), _address(address) {}

  bool begin() {
    if (_pins.scl >= 0 && _pins.sda >= 0) {
#if defined(ARDUINO_ARCH_ESP32)
      _wire.begin((int)_pins.sda, (int)_pins.scl);
#else
      _wire.begin();
#endif
    } else {
      _wire.begin();
    }

    if (_pins.pwdn >= 0) {
      pinMode((uint8_t)_pins.pwdn, OUTPUT);
      digitalWrite((uint8_t)_pins.pwdn, LOW);
      delay(5);
    }

    if (_pins.ret >= 0) {
      pinMode((uint8_t)_pins.ret, OUTPUT);
      digitalWrite((uint8_t)_pins.ret, HIGH);
      delay(10);
    }

    if (_pins.vs >= 0) pinMode(_pins.vs, INPUT);
    if (_pins.hs >= 0) pinMode(_pins.hs, INPUT);
    if (_pins.pclk >= 0) pinMode(_pins.pclk, INPUT);
    
    int dPins[8] = {_pins.d0, _pins.d1, _pins.d2, _pins.d3, _pins.d4, _pins.d5, _pins.d6, _pins.d7};
    for (int i = 0; i < 8; i++) {
      if (dPins[i] >= 0) pinMode(dPins[i], INPUT);
    }

    delay(10);
    uint8_t productId = 0;
    if (!readRegister(0x0A, productId)) {
      _ready = false;
      return false;
    }
    
    _productId = productId;
    _ready = (productId == 0x76);
    return _ready;
  }

  bool writeRegister(uint8_t reg, uint8_t value) {
    _wire.beginTransmission(_address);
    _wire.write(reg);
    _wire.write(value);
    return _wire.endTransmission() == 0;
  }

  bool readRegister(uint8_t reg, uint8_t &value) {
    _wire.beginTransmission(_address);
    _wire.write(reg);
    if (_wire.endTransmission(false) != 0) return false;
    if (_wire.requestFrom((int)_address, 1) != 1) return false;
    value = (uint8_t)_wire.read();
    return true;
  }

  bool resetSensor() {
    if (!writeRegister(0x12, 0x80)) return false;
    delay(50);
    return true;
  }

  void configureQVGA() {
    resetSensor();
    writeRegister(0x12, 0x14); 
    writeRegister(0x40, 0xD0); 
    writeRegister(0x8C, 0x00); 
    writeRegister(0x11, 0x01); 
    writeRegister(0x0C, 0x00);
    writeRegister(0x0D, 0x00);
    writeRegister(0x14, 0x18); 
    writeRegister(0x70, 0x3A); 
    writeRegister(0x71, 0x35); 
    writeRegister(0x72, 0x11); 
    writeRegister(0x73, 0x00);
    writeRegister(0x74, 0x20);
    writeRegister(0x75, 0x80);
    writeRegister(0x76, 0x01);
    writeRegister(0x17, 0x16);
    writeRegister(0x18, 0x04);
    writeRegister(0x19, 0x02);
    writeRegister(0x1A, 0x7A);
    writeRegister(0x32, 0x80);
  }

  bool readFrame(uint16_t *frameBuffer) {
    if (!_ready || !frameBuffer) return false;
    uint32_t timeout = millis();

    while (digitalRead(_pins.vs) == HIGH) {
      if (millis() - timeout > 250) return false;
    }
    while (digitalRead(_pins.vs) == LOW) {
      if (millis() - timeout > 250) return false;
    }

    int dPins[8] = {_pins.d0, _pins.d1, _pins.d2, _pins.d3, _pins.d4, _pins.d5, _pins.d6, _pins.d7};

    for (int y = 0; y < 240; y++) {
      uint32_t lineTimeout = millis();
      while (digitalRead(_pins.hs) == LOW) {
        if (millis() - lineTimeout > 100) return false;
      }
      
      for (int x = 0; x < 320; x++) {
        while (digitalRead(_pins.pclk) == LOW) {
          if (digitalRead(_pins.hs) == LOW) return false;
        }
        
        uint8_t high_byte = 0;
        for (int i = 0; i < 8; i++) {
          if (digitalRead(dPins[i])) high_byte |= (1 << i);
        }

        while (digitalRead(_pins.pclk) == HIGH);
        while (digitalRead(_pins.pclk) == LOW);
        
        uint8_t low_byte = 0;
        for (int i = 0; i < 8; i++) {
          if (digitalRead(dPins[i])) low_byte |= (1 << i);
        }

        frameBuffer[y * 320 + x] = (low_byte << 8) | high_byte;

        while (digitalRead(_pins.pclk) == HIGH);
      }
      while (digitalRead(_pins.hs) == HIGH);
    }
    return true;
  }

  bool ready() const { return _ready; }
  uint8_t productId() const { return _productId; }

private:
  OV7670Pins _pins;
  TwoWire &_wire;
  uint8_t _address;
  uint8_t _productId = 0;
  bool _ready = false;
};

} // namespace SUI
""",
    "src/main.cpp": """#include <Arduino.h>
#include <Wire.h>
#define SUI_ENABLE_OV7670
#include <SashaUniversalIO.h>

SUI::OV7670Pins cameraPins = {
  .scl = 22,  .sda = 21,  .vs = 25,   .hs = 26,   .pclk = 27, .xclk = 16,
  .d7 = 12, .d6 = 34, .d5 = 13, .d4 = 14,
  .d3 = 19, .d2 = 39, .d1 = 36, .d0 = 35,
  .ret = 4,   .pwdn = 17
};

SUI::OV7670Camera ov7670(cameraPins);
uint16_t *frameBuffer = nullptr;
bool capturingEnabled = true;

void setup() {
  Serial.begin(921600);
  while (!Serial);

  frameBuffer = (uint16_t*) malloc(320 * 240 * sizeof(uint16_t));
  if (!frameBuffer) return;

  // Замедляем XCLK до 1 МГц, чтобы ESP32 стабильно успевал читать шину
  ledcAttach(16, 1000000, 3);
  ledcWrite(16, 4);

  if (ov7670.begin()) {
    ov7670.configureQVGA();
    Serial.println("CAM_OK");
  } else {
    Serial.println("CAM_FAIL");
  }
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == '1') capturingEnabled = true;
    else if (cmd == '0') capturingEnabled = false;
  }

  if (capturingEnabled && frameBuffer) {
    if (ov7670.readFrame(frameBuffer)) {
      Serial.write(0xAA);
      Serial.write(0x55);
      Serial.write((uint8_t*)frameBuffer, 320 * 240 * sizeof(uint16_t));
    }
  } else {
    delay(50);
  }
}
""",
    "ffc.py": """import cv2
import numpy as np
import serial
from serial.tools import list_ports

BAUD = 921600
WIDTH = 320
HEIGHT = 240
FRAME_SIZE = WIDTH * HEIGHT * 2

ports = list_ports.comports()
PORT = None
for p in ports:
    if "CP210" in p.description or "CH340" in p.description or "USB" in p.device:
        PORT = p.device
        break

if not PORT and ports:
    PORT = ports[0].device

if not PORT:
    print("❌ Ошибка: Не найдены COM-порты!")
    exit(1)

print(f"Подключаемся к порту {PORT} на скорости {BAUD}...")

try:
    ser = serial.Serial(PORT, BAUD, timeout=1)
except Exception as e:
    print(f"❌ Ошибка открытия порта {PORT}: {e}")
    exit(1)

print("Успешно подключено! Ожидание кадра...")

try:
    while True:
        b1 = ser.read()
        if not b1:
            continue
        if ord(b1) == 0xAA:
            b2 = ser.read()
            if b2 and ord(b2) == 0x55:
                frame_data = ser.read(FRAME_SIZE)
                if len(frame_data) == FRAME_SIZE:
                    raw_np = np.frombuffer(frame_data, dtype=np.uint16).reshape((HEIGHT, WIDTH))
                    
                    r = ((raw_np & 0xF800) >> 8).astype(np.uint8)
                    g = ((raw_np & 0x07E0) >> 3).astype(np.uint8)
                    b = ((raw_np & 0x001F) << 3).astype(np.uint8)
                    img = np.stack([b, g, r], axis=-1)
                    
                    cv2.imshow("ESP32 OV7670 Stream", img)
                    if cv2.waitKey(1) & 0xFF == ord('q'):
                        break
except KeyboardInterrupt:
    print("\\nВыход...")
finally:
    ser.close()
    cv2.destroyAllWindows()
""",
}


def get_hash(text):
  normalized = text.replace("\r\n", "\n").strip().encode("utf-8")
  return hashlib.md5(normalized).hexdigest()


def check_and_update():
  print("==> Синхронизация базы проекта через setup_project.py...")

  for filepath, ideal_content in FILES.items():
    directory = os.path.dirname(filepath)
    if directory and not os.path.exists(directory):
      os.makedirs(directory, exist_ok=True)
      print(f"[+] Создана папка: {directory}")

    ideal_hash = get_hash(ideal_content)
    file_exists = os.path.exists(filepath)
    should_write = False
    reason = ""

    if not file_exists:
      should_write = True
      reason = "файл отсутствует"
    else:
      try:
        with open(filepath, "r", encoding="utf-8") as f:
          current_content = f.read()
        if get_hash(current_content) != ideal_hash:
          should_write = True
          reason = "обновление версии"
      except Exception:
        should_write = True
        reason = "ошибка чтения"

    if should_write:
      with open(filepath, "w", encoding="utf-8") as f:
        f.write(ideal_content.strip() + "\n")
      print(f"[⚡] Обновлен ({reason}): {filepath}")
    else:
      print(f"[✓] Актуален: {filepath}")

  print(
      "\n==> Всё на базе! Выполняй 'pio run --target upload', а после прошивки"
      " запускай 'python ffc.py'"
  )


if __name__ == "__main__":
  check_and_update()
