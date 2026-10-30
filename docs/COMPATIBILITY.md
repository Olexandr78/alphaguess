# Compatibility and hardware notes

## Character LCD

`SUI_I2CLCD.h` currently targets HD44780-compatible character displays using the common PCF8574 backpack mapping:

- P0 RS
- P1 RW
- P2 Enable
- P3 Backlight
- P4..P7 D4..D7

Common I2C addresses are 0x27 and 0x3F, but scan your bus instead of assuming. Some backpack boards use a different pin mapping and need a configurable mapping driver (planned).

## TFT

`SUI_AdafruitTFT.h` is a small API adapter, not a controller driver. It expects a display object compatible with Adafruit_GFX. Install and initialize the controller library separately.

| Controller family | Typical Arduino driver |
|---|---|
| ST7735 | Adafruit_ST7735 |
| ST7789 | Adafruit_ST7789 |
| ILI9341 | Adafruit_ILI9341 |
| ILI9488 | A compatible ILI9488 driver exposing Adafruit_GFX |

ILI9486 and parallel/RGB panels vary by driver and board. They are not guaranteed by this adapter. SSD1306 is an OLED controller, not a TFT controller.

## Servo

Include `SUI_Servo.h` and install the Arduino Servo library. Servos need an adequate external 5 V supply in many projects. Connect grounds together; never power a larger servo from an I/O pin.

## OV7670 camera

OV7670 is a camera sensor, not a display. It outputs pixel data and requires sensor configuration over SCCB (I2C-like), clocking and high-speed pixel capture. Many classic AVR Arduino boards lack enough RAM and capture bandwidth for useful full-frame capture. A universal implementation would be misleading; a proper driver must target a specific board/capture method (for example FIFO-equipped modules or MCUs with a camera/DMA interface). This module remains on the roadmap.

## Voltage and wiring

- Check the module's VCC and logic voltage before connecting it.
- Many TFT modules use 3.3 V logic and are not 5 V tolerant.
- Relay modules can switch dangerous mains voltage. Use a properly enclosed, rated module and get qualified help; do not handle exposed mains wiring.
- Use external supplies for motors/servos and connect grounds where required.
