#pragma once
#include <Arduino.h>
#include <SD.h>

namespace SUI {
class SDCard {
public:
  bool begin(uint8_t chipSelectPin) { return SD.begin(chipSelectPin); }
  bool exists(const char *path) { return SD.exists(path); }
  bool remove(const char *path) { return SD.remove(path); }
  bool mkdir(const char *path) { return SD.mkdir(path); }

#if defined(ARDUINO_ARCH_ESP32)
  // ESP32 FS uses string modes ("r", "w", "a"), unlike AVR SD's bit flags.
  File open(const char *path, const char *mode=FILE_READ) {
    return SD.open(path, mode);
  }
#else
  // AVR SD library uses FILE_READ / FILE_WRITE bit flags.
  File open(const char *path, uint8_t mode=FILE_READ) {
    return SD.open(path, mode);
  }
#endif
};
}
