# SashaUniversalIO

**One Arduino library and one main include for common displays, sensors, motors, communication modules and I/O hardware.**

> The repository is still named `alphaguess`; the connected GitHub integration cannot rename repositories. The existing AlphaGuess release and APK are untouched.

## Quick start

```cpp
#include <SashaUniversalIO.h>

SUI::Relay relay(7, SUI::ActiveLevel::LOW);
SUI::Joystick stick(A0, A1, 2);
SUI::RotaryEncoder encoder(3, 4, 5);
SUI::ActiveBeeper beeper(6);

void setup() {
  relay.begin();
  stick.begin();
  encoder.begin();
  beeper.begin();
}

void loop() {
  encoder.update();
  if (stick.pressed()) relay.toggle();
  if (encoder.position() > 10) beeper.on();
  else beeper.off();
}
```

## Modules in the main header

| Device / module | API | Notes |
|---|---|---|
| Relay modules | `SUI::Relay` | Active HIGH/LOW; relay contacts and load wiring are external |
| Buttons | `SUI::Button` | Pull-up or plain input |
| Passive buzzer | `SUI::Buzzer` / `SUI::PassiveBeeper` | Uses `tone()` |
| Active beeper | `SUI::ActiveBeeper` | Digital on/off |
| HC-SR04 | `SUI::HCSR04` | Ultrasonic distance |
| HW-504 / KY-023 joystick | `SUI::Joystick` | X, Y analog values and SW button |
| HW-040 / KY-040 encoder | `SUI::RotaryEncoder` | CLK, DT, optional SW; call `update()` often |
| 4-wire stepper | `SUI::Stepper4Wire` | Basic blocking sequence for ULN2003/L298N-style inputs; not a precision motion controller |
| Single 7-segment digit | `SUI::SevenSegment` | Common-anode or common-cathode; segment resistors required |
| DIP switch bank | `SUI::DipSwitch` | Up to 8 switches, INPUT_PULLUP |
| Analog microphone | `SUI::AnalogMicrophone` | For amplified AO microphone boards |
| Digital sound detector | `SUI::SoundDetector` | LM393-style DO output |
| HC-05 / HC-06 | `SUI::BluetoothUART` | Wraps a `Stream`; use HardwareSerial or SoftwareSerial |
| HW-748 / SIM800L | `SUI::SIM800L` | Basic AT-command UART helper only; not a full GSM API |

## Optional adapters

These adapters use third-party Arduino libraries and are kept in separate headers so the main include still compiles when those libraries are not installed.

| Device | Header | Dependency |
|---|---|---|
| HD44780 1602/2004 I2C LCD | `SUI_I2CLCD.h` | Wire (included with Arduino); common PCF8574 backpack mapping |
| TFT (ST7735/ST7789/ILI9341 and compatible) | `SUI_AdafruitTFT.h` | Adafruit_GFX plus the controller driver |
| Servo | `SUI_Servo.h` | Arduino Servo |
| SD / microSD | `SUI_SD.h` | Arduino SD |
| DFPlayer Mini | `SUI_DFPlayer.h` | DFRobotDFPlayerMini |

Example:
```cpp
#include <SashaUniversalIO.h>
#include <SUI_Servo.h>
#include <SUI_SD.h>
#include <SUI_DFPlayer.h>
```

## Displays

- Character LCD: common HD44780 with PCF8574 I2C backpack.
- TFT: thin adapter for Adafruit_GFX-compatible controller libraries. Install the correct controller library separately.
- Seven-segment: direct GPIO single digit in the main header. TM1637/MAX7219 and multi-digit multiplexed modules require their own controller adapter.
- OLED is a separate display family, not a TFT.

## Cameras and hardware-specific modules

OV7670 is **not yet implemented**. It needs SCCB register configuration, a pixel clock and fast capture; classic AVR boards often lack RAM/bandwidth. Support must be developed and tested per MCU/capture hardware.

## Electrical safety and compatibility

- Check the exact module revision and pinout. Board names are often reused by different vendors.
- HC-05/HC-06 and SIM800L UART logic levels differ by breakout revision. Do not assume RX pins are 5 V tolerant; use level shifting where needed.
- SIM800L needs a stable supply capable of high current bursts; do not power it from an Arduino 3.3 V pin.
- Microphone capsules need bias and amplification. The analog class expects an already amplified AO output; it does not connect a bare electret capsule directly.
- Stepper motors and servos need suitable motor supplies and drivers; never power them from GPIO.
- SD cards use 3.3 V logic; use a level-shifted module with 5 V Arduino boards.
- TFT modules may also be 3.3 V only.
- Relays may switch hazardous mains voltage. Use enclosed, properly rated hardware and qualified help; never touch exposed mains wiring.

## Current limitations

This is a growing library, not a magical driver for every chip ever made. Some device families need their own controller libraries or board-specific implementations. Hardware compilation and physical tests are still required; no universal hardware test has been claimed.

## License

MIT. See [LICENSE](LICENSE).
