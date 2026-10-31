# SashaUniversalIO

**Arduino toolkit with one main include for built-in modules and optional adapters for external driver libraries.**

The repository is still named `alphaguess`; its AlphaGuess APK and release are untouched. Library metadata and code are at repository root and `src/`.

## Quick start

```cpp
#include <SashaUniversalIO.h>

SUI::Joystick joystick(A0, A1, 2);       // HW-504
SUI::RotaryEncoder encoder(3, 4, 5);    // HW-040
SUI::Relay relay(7, SUI::ActiveLevel::LOW_LEVEL);
SUI::Stepper4Wire motor(8, 9, 10, 11);
SUI::TM1637Display display(12, 13);

void setup() {
  joystick.begin(); encoder.begin(); relay.begin();
  motor.begin(); display.begin();
}
void loop() {
  encoder.update();
  display.showNumber((int)encoder.position());
  if (joystick.pressed()) relay.toggle();
}
```

## Built-in drivers (available from the one main include)

| Device family | Class | Notes |
|---|---|---|
| Relay, button, buzzer | `Relay`, `Button`, `Buzzer`, `ActiveBeeper` | Active HIGH/LOW relay; passive tone and active on/off buzzers |
| Ultrasonic | `HCSR04` | HC-SR04 |
| HW-504 / KY-023 | `Joystick` | Two analog axes + push switch |
| HW-040 / KY-040 | `RotaryEncoder` | Quadrature CLK/DT + optional SW |
| Generic sensor input | `AnalogInput`, `DigitalInput` | Analog and digital output modules |
| Debounced switch | `DebouncedButton` | Non-blocking debounce with press/release events |
| PWM | `PWMOutput` | Basic 8-bit analogWrite wrapper |
| 4-wire stepper | `Stepper4Wire` | Basic 8-phase output through a driver |
| STEP/DIR stepper | `StepDirStepper` | A4988/DRV8825/TMC-compatible pulse interface |
| Single 7-segment | `SevenSegment` | Common anode/cathode, direct GPIO |
| TM1637 | `TM1637Display` | 4-digit display and brightness |
| MAX7219/MAX7221 | `MAX7219Display` | Segment digits and raw register output |
| 74HC595 | `ShiftRegister595` | 8 latched digital outputs |
| DIP switches | `DipSwitch` | Up to 8 pull-up inputs |
| Microphone modules | `AnalogMicrophone`, `SoundDetector` | Amplified AO / digital DO; bare electret needs a preamp |
| HC-05 / HC-06 | `BluetoothUART` | HardwareSerial or SoftwareSerial Stream wrapper |
| HW-748 / SIM800L | `SIM800L` | Raw AT serial helper, not full cellular stack |
| Character LCD | `I2CLCD`, `ParallelLCD` | HD44780 via common PCF8574 backpack or 4-bit GPIO |

## Optional adapters in the same library package

Define the switch before `#include <SashaUniversalIO.h>`. Install the dependency via Arduino Library Manager.

| Define | Class / adapter | Dependency |
|---|---|---|
| `SUI_ENABLE_SERVO` | `ServoMotor` | Arduino Servo |
| `SUI_ENABLE_TFT` | `AdafruitTFT` | Adafruit_GFX + matching controller driver (legacy adapter) |
| `SUI_ENABLE_ST7735S` | `ST7735S` | Native SPI driver for ST7735S; no Adafruit dependency |
| `SUI_ENABLE_OLED` | `AdafruitOLED` | Adafruit_GFX + Adafruit SSD1306 |
| `SUI_ENABLE_SD` | `SDCard` | Arduino SD |
| `SUI_ENABLE_DFPLAYER` | `DFPlayer` | DFRobotDFPlayerMini |
| `SUI_ENABLE_OV7670` | `OV7670Camera`, `OV7670Pins` | Native SCCB register control via Arduino Wire; no camera library dependency |
| `SUI_ENABLE_DHT` | `DHTSensor` | DHT sensor library |
| `SUI_ENABLE_DS18B20` | `DS18B20` | OneWire + DallasTemperature |
| `SUI_ENABLE_BMP280` | `BMP280Sensor` | Adafruit BMP280 Library |

Example:

```cpp
#define SUI_ENABLE_SD
#define SUI_ENABLE_DFPLAYER
#define SUI_ENABLE_OLED
#include <SashaUniversalIO.h>
```

The main header stays buildable without optional third-party libraries when their switches are not defined. On ESP32, install ESP32Servo for SUI_ENABLE_SERVO; on classic AVR boards, install the standard Servo library. For a native ST7735S display, define `SUI_ENABLE_ST7735S` before the main include and use `SUI::ST7735S`; it draws pixels, lines, rectangles, circles and basic ASCII text directly over SPI without Adafruit. The older `SUI_ENABLE_TFT` adapter remains available for other Adafruit_GFX-compatible controllers.

## OV7670 notes

The native `OV7670Camera` driver configures SCCB through Arduino `Wire`, reads the sensor product ID, and exposes register read/write plus software reset. `OV7670Pins` records the module pinout: 3.3V, DGND, SCL, SDA, VS, HS, PLK/PCLK, XLK/XCLK, D7-D0, RET and PWDN. Power and ground are documentation-only fields. `RET` is preserved as labelled and is not toggled automatically because its function depends on the module revision. On ESP32, custom SCL/SDA pins are passed to `Wire.begin`; on other cores, `Wire.begin()` uses the board's hardware-I2C pins. **Pixel capture is not implemented by this SCCB layer yet**: it needs a board-specific clock and parallel/DMA backend. Do not interpret successful sensor detection as proof that frame capture works. The Uno/Nano AVR is not a practical full-frame capture target due to its limited SRAM and throughput.

## Electrical notes

- Check exact PCB revision and pinout; board labels are not always consistent.
- Motors and servos need suitable drivers and power supplies, never GPIO power. Configure STEP/DIR driver current limits.
- Use one current-limiting resistor per directly driven seven-segment LED segment.
- A bare electret microphone needs bias and a preamplifier.
- HC-05/HC-06 and SIM800L UART logic levels vary; use level shifting where required. HW-748 is sold as a SIM800L breakout/kit; SIM800L requires a stable high-current supply.
- SD cards, OLED and TFT modules may use 3.3V logic; check level shifting.
- Relays may switch lethal mains voltage. Use enclosed, rated equipment and qualified supervision.

## Examples

`BasicModules`, `I2C_LCD`, `Parallel_LCD`, `ServoMotor`, `TFT_ST7735S_SUI`, `TFT_AdafruitGFX`, `OLED_SSD1306`, `Joystick_Encoder`, `SD_Card`, `Stepper_SevenSegment_Audio`, `DisplayControllers`, `OV7670_Capture`, `DFPlayer_Mini`, `Environmental_Sensors`.

## Build and test status

A GitHub Actions workflow compiles the Uno examples, an ESP32 Dev Module-compatible example set, and the OV7670 example for Nano 33 BLE. The Servo adapter selects ESP32Servo automatically on ESP32. These automated builds are not physical hardware tests. No claim is made that every module revision or every Arduino-compatible chip is supported; report tested board/module combinations to expand the compatibility matrix.

## License

MIT. See [LICENSE](LICENSE).
