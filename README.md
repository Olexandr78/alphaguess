# SashaUniversalIO

**An expandable Arduino hardware toolkit with one main include for built-in drivers and optional switches for third-party adapters.**

The repository is still named `alphaguess`; the AlphaGuess APK and release are preserved. The library source is under `src/`.

## One include for the built-in drivers

```cpp
#include <SashaUniversalIO.h>

SUI::Joystick joystick(A0, A1, 2);       // HW-504
SUI::RotaryEncoder encoder(3, 4, 5);    // HW-040
SUI::Relay relay(7, SUI::ActiveLevel::LOW);
SUI::Stepper4Wire stepper(8, 9, 10, 11);
SUI::TM1637Display display(12, 13);
```

The main header includes core I/O, common modules, display controllers and the I2C character LCD helper.

## Built-in drivers

| Family / module | Class | Capabilities |
|---|---|---|
| Relay | `Relay` | Active HIGH/LOW, on/off/toggle |
| Buttons | `Button` | Pull-up or plain digital input |
| Passive / active beepers | `Buzzer`, `ActiveBeeper` | Tone playback or digital on/off |
| Ultrasonic | `HCSR04` | HC-SR04 distance in cm |
| HW-504 / KY-023 | `Joystick` | X/Y analog axes and push button |
| HW-040 / KY-040 | `RotaryEncoder` | Quadrature CLK/DT and optional SW |
| 4-wire stepper | `Stepper4Wire` | Basic 8-phase sequence for driver inputs |
| STEP/DIR stepper | `StepDirStepper` | A4988/DRV8825/TMC STEP/DIR interface |
| Single 7-segment | `SevenSegment` | Common-anode/cathode direct GPIO |
| TM1637 | `TM1637Display` | 4-digit display, brightness and number |
| MAX7219/MAX7221 | `MAX7219Display` | 8-digit/LED segment controller, raw segments and digits |
| 74HC595 | `ShiftRegister595` | 8 latched digital outputs |
| DIP switch | `DipSwitch` | Up to 8 pull-up inputs |
| Analog mic / sound module | `AnalogMicrophone`, `SoundDetector` | Amplified analog AO / comparator digital DO |
| HC-05 / HC-06 | `BluetoothUART` | Generic UART `Stream` wrapper |
| HW-748 / SIM800L | `SIM800L` | AT-command UART transport, not a full GSM stack |
| HD44780 I2C LCD | `I2CLCD` | Common PCF8574 backpack mapping |

## Optional adapters (same library package)

Enable an adapter before the main include. Install its dependency through Arduino Library Manager first.

| Define | Adapter | Dependency |
|---|---|---|
| `SUI_ENABLE_SERVO` | `ServoMotor` | Arduino Servo |
| `SUI_ENABLE_TFT` | `AdafruitTFT` | Adafruit_GFX + matching controller driver |
| `SUI_ENABLE_SD` | `SDCard` | Arduino SD |
| `SUI_ENABLE_DFPLAYER` | `DFPlayer` | DFRobotDFPlayerMini |
| `SUI_ENABLE_OV7670` | `OV7670Camera` | Arduino_OV767X |

Example:

```cpp
#define SUI_ENABLE_DFPLAYER
#define SUI_ENABLE_SD
#include <SashaUniversalIO.h>
```

For TFTs, include the controller's own header and pass its display object to `SUI::AdafruitTFT<Display>`.

## OV7670 support and limitations

The OV7670 adapter wraps the official Arduino_OV767X driver rather than pretending that camera capture is identical on every MCU. The upstream driver supports OV7670/OV7675 on selected boards; its documented example uses Nano 33 BLE, QVGA/RGB565 and a large frame buffer. This is not suitable for an Uno/Nano AVR with its small SRAM. See the [upstream driver](https://github.com/arduino-libraries/Arduino_OV767X) and [capture example](https://github.com/arduino-libraries/Arduino_OV767X/blob/master/examples/CameraCaptureRawBytes/CameraCaptureRawBytes.ino).

## Hardware and electrical notes

- HW-504 uses two analog potentiometer outputs plus a push switch. HW-040 is a quadrature encoder.
- 4-wire stepper motors must connect through a suitable driver (e.g. ULN2003). STEP/DIR motors need a driver such as A4988/DRV8825; configure its current limit and pulse timing.
- Seven-segment direct GPIO needs one current-limiting resistor per segment. TM1637/MAX7219 modules have their own controller chips.
- A bare electret capsule requires bias and a preamplifier. `AnalogMicrophone` expects an amplified AO signal.
- HC-05/HC-06 and SIM800L logic levels vary by breakout; use level shifting when required. HW-748 is sold as a SIM800L breakout/kit. SIM800L needs a stable supply for current bursts; never power it from a GPIO/3.3V pin.
- SD cards and many TFT modules use 3.3V logic; check the board's level shifting.
- Servos, motors, SIM800L and relays need appropriate power circuitry. Relays may switch lethal mains voltage: use enclosed, rated equipment and qualified supervision.
- Module labels and clones vary. Check the actual PCB pinout before wiring.

## Examples

- `BasicModules`
- `I2C_LCD`
- `ServoMotor`
- `TFT_AdafruitGFX`
- `Joystick_Encoder`
- `SD_Card`
- `Stepper_SevenSegment_Audio`
- `DisplayControllers`
- `OV7670_Capture`
- `DFPlayer_Mini`

## Build status

The library now contains implementations for the modules listed above, but it has not been physically tested against every board or module revision. The OV767X upstream library itself is board-specific. This project does not claim compatibility with every Arduino chip, display controller or sensor on the market. Report tested board + module combinations to grow the compatibility matrix.

## License

MIT. See [LICENSE](LICENSE).
