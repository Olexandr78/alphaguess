import os
import shutil
import zipfile

# Имя твоего исходного архива с библиотекой
ZIP_FILE = "alphaguess-main (1).zip"
EXTRACT_DIR = "SashaUniversalIO_patched"

# Полный пропатченный код для SUI_OV7670.h
OV7670_PATCH_CONTENT = """#pragma once

#include <Arduino.h>
#include <Wire.h>

namespace SUI {

// Стандартные разрешения и форматы для OV7670
enum CameraResolution {
  RES_QQVGA = 0, // 160x120
  RES_QVGA  = 1, // 320x240
  RES_VGA   = 2  // 640x480
};

enum CameraFormat {
  FORMAT_RGB565 = 0,
  FORMAT_YUV422 = 1
};

struct OV7670Pins {
  int8_t vcc = -1;   // Не подключать к GPIO (только питание 3.3V)
  int8_t dgnd = -1;  // На землю (GND)
  int8_t scl = -1;   // I2C SCL (SIOC)
  int8_t sda = -1;   // I2C SDA (SIOD)
  int8_t vs = -1;    // VSYNC
  int8_t hs = -1;    // HREF
  int8_t pclk = -1;  // PCLK (Pixel Clock)
  int8_t xclk = -1;  // XCLK (Master Clock от ESP32 LEDC)
  int8_t d7 = -1;    // Данные D7
  int8_t d6 = -1;    // Данные D6
  int8_t d5 = -1;    // Данные D5
  int8_t d4 = -1;    // Данные D4
  int8_t d3 = -1;    // Данные D3
  int8_t d2 = -1;    // Данные D2
  int8_t d1 = -1;    // Данные D1
  int8_t d0 = -1;    // Данные D0
  int8_t ret = -1;   // RESET
  int8_t pwdn = -1;  // PWDN
};

class OV7670Camera {
public:
  explicit OV7670Camera(const OV7670Pins &pins, TwoWire &wire = Wire, uint8_t address = 0x21)
      : _pins(pins), _wire(wire), _address(address) {}

  // Инициализация SCCB (I2C) интерфейса и проверка Product ID камеры (должен быть 0x76)
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

  void end() {
    _ready = false;
    _wire.end();
  }

  bool ready() const { return _ready; }
  uint8_t productId() const { return _productId; }
  const OV7670Pins &pins() const { return _pins; }

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

  // Аппаратный сброс регистров камеры
  bool resetSensor() {
    if (!writeRegister(0x12, 0x80)) return false;
    delay(50);
    return true;
  }

  // Готовый профиль настройки камеры на разрешение QVGA (320x240) в формате RGB565
  bool setConfigQVGA_RGB565() {
    if (!_ready) return false;

    resetSensor();

    // COM7: Режим QVGA + RGB
    writeRegister(0x12, 0x14); 
    // COM15: Формат RGB565 [11000000] -> D7, D6 = 1 (включает RGB565)
    writeRegister(0x40, 0xD0); 
    writeRegister(0x8C, 0x00); 
    
    writeRegister(0x11, 0x01); // Clock divider
    writeRegister(0x0C, 0x00);
    writeRegister(0x0D, 0x00);
    writeRegister(0x14, 0x18); // Scaling / Limit
    
    // Автоматическая экспозиция, баланс белого и цвета
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

    return true;
  }

private:
  OV7670Pins _pins;
  TwoWire &_wire;
  uint8_t _address;
  uint8_t _productId = 0;
  bool _ready = false;
};

} // namespace SUI
"""


def main():
  if not os.path.exists(ZIP_FILE):
    print(f"Ошибка: Не найден архив {ZIP_FILE} в текущей папке!")
    return

  print(f"1. Распаковываем {ZIP_FILE}...")
  if os.path.exists(EXTRACT_DIR):
    shutil.rmtree(EXTRACT_DIR)
  with zipfile.ZipFile(ZIP_FILE, "r") as z:
    z.extractall(EXTRACT_DIR)
  print(f"Архив успешно распакован в папку: {EXTRACT_DIR}")

  print("2. Применяем патчи (обновляем SUI_OV7670.h)...")
  target_file = None
  for root, dirs, files in os.walk(EXTRACT_DIR):
    for file in files:
      if file == "SUI_OV7670.h":
        target_file = os.path.join(root, file)
        break

  if target_file:
    with open(target_file, "w", encoding="utf-8") as f:
      f.write(OV7670PATCH_CONTENT)
    print(f"Патч успешно применен к файлу: {target_file}")
  else:
    print("Предупреждение: Файл SUI_OV7670.h не найден внутри архива.")

  print("\nГотово! Проект пропатчен и готов к использованию.")


if __name__ == "__main__":
  main()
