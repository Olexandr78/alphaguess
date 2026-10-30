# SashaUniversalIO

**A modular Arduino library for displays, sensors and common hardware modules.**

> The repository name is still `alphaguess` because the connected GitHub tools cannot rename repositories. The existing AlphaGuess 1.0 release and its APK are left untouched.

SashaUniversalIO aims to give projects a consistent API for common Arduino hardware. Hardware with different protocols still needs its own driver: there is no single electrical or software protocol shared by every LCD, TFT controller or camera.

## What works in this first foundation

| Module | Status | Notes |
|---|---|---|
| Relay modules | Ready | Active-high or active-low, one or more instances |
| Buttons | Ready | Pull-up input and pressed-state reading |
| Buzzer | Ready | Tone and silence |
| HC-SR04 | Ready | Distance in cm |
| I2C character LCD | Ready | HD44780-compatible 1602/2004 with common PCF8574 backpack mapping |
| Servo | Optional adapter | Uses Arduino Servo library |
| TFT displays | Optional adapter | Adafruit_GFX-compatible drivers; install the controller library separately |
| OV7670 camera | Planned / hardware-specific | Capture timing, FIFO and RAM requirements differ by board; not advertised as universally supported |
| DHT / DS18B20 / BMP280 | Planned adapters | Separate sensor libraries will be used |

## Supported TFT route

The optional `SUI_AdafruitTFT.h` adapter works with displays whose driver exposes the Adafruit_GFX API. Depending on the module, install one of:
- Adafruit ST7735 and ST7789 Library
- Adafruit ILI9341
- Adafruit ILI9488 (third-party compatible driver)

The adapter does not magically make every controller compatible. Check the exact controller, resolution, interface (SPI/parallel), voltage and pinout.

## Install

1. Download this repository as ZIP.
2. In Arduino IDE choose **Sketch → Include Library → Add .ZIP Library**.
3. Select the ZIP file.
4. Include `SashaUniversalIO.h` in your sketch.

Some optional adapters need their own Arduino libraries. See `docs/COMPATIBILITY.md`.

## Quick example

```cpp
#include <SashaUniversalIO.h>

SUI::Relay lamp(7, SUI::ActiveLevel::LOW);
SUI::Button button(2);
SUI::Buzzer buzzer(8);
SUI::HCSR04 sonar(10, 11);

void setup() {
  Serial.begin(9600);
  lamp.begin();
  button.begin();
  buzzer.begin();
  sonar.begin();
}

void loop() {
  if (button.isPressed()) {
    lamp.toggle();
    buzzer.beep(1200, 80);
  }

  Serial.print("Distance: ");
  Serial.print(sonar.distanceCm());
  Serial.println(" cm");
  delay(100);
}
```

## Examples

Open **File → Examples → SashaUniversalIO** in Arduino IDE:
- `BasicModules`
- `I2C_LCD`
- `ServoMotor`
- `TFT_AdafruitGFX`

## Roadmap

- More I2C LCD backpack pin mappings
- ST7735 / ST7789 / ILI9341 / ILI9486 / ILI9488 tested examples
- OLED SSD1306 (OLED, not TFT)
- DHT11 / DHT22, DS18B20, BMP280, rotary encoder and motor drivers
- OV7670 capture adapters for specific boards with enough RAM and camera interface support
- Board-by-board compatibility tests

## Important

Check voltage levels before wiring. Many TFT modules are **3.3 V logic** and may be damaged by 5 V signals. Do not power servos or motors from an Arduino I/O pin. Use a suitable external supply and a shared ground.

## License

MIT. See [LICENSE](LICENSE).
