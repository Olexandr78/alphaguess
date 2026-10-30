# Compatibility, setup and wiring

## HW-504 joystick
VRx and VRy connect to analog inputs; SW is a momentary switch. `SUI::Joystick(xPin,yPin,swPin)` uses INPUT_PULLUP for SW. Example: A0, A1, D2.

## HW-040 rotary encoder
CLK, DT and optional SW connect to digital inputs. `SUI::RotaryEncoder(clk,dt,sw)` uses quadrature transition decoding; call `update()` frequently in loop(). Mechanical bounce and wiring quality can still affect readings.

## Stepper motors
- `Stepper4Wire` is a basic blocking 8-phase output sequence for a suitable four-input driver board (e.g. ULN2003).
- `StepDirStepper` emits STEP/DIR pulses for A4988, DRV8825 or compatible drivers. Configure driver current limit, microstep pins and pulse timing. No acceleration planning or stall detection is implemented.

Never connect a motor winding directly to MCU pins.

## Displays
- `SevenSegment` controls one bare digit directly; order is a,b,c,d,e,f,g,dp. Use a resistor per segment.
- `TM1637Display` handles common 4-digit TM1637 boards.
- `MAX7219Display` handles MAX7219/MAX7221 serial segment modules. It writes raw segment data/digits; it is not a graphical matrix font renderer.
- `I2CLCD` targets HD44780 LCDs with a common PCF8574 backpack pin mapping (P0=RS, P1=RW, P2=EN, P3=backlight, P4..P7=D4..D7). Other backpack mappings need a custom driver.
- TFT support uses an Adafruit_GFX-compatible controller library; the helper is not a controller driver.

## Microphones and beepers
The analog microphone helper reads an already amplified analog output. A bare electret capsule requires bias and a preamplifier. A digital sound module's DO output can be read using `SoundDetector`. Active beepers are digital on/off; passive piezo buzzers use `tone()`.

## Bluetooth and GSM
`BluetoothUART` wraps any Arduino `Stream`; pass HardwareSerial or SoftwareSerial. It does not configure pairing or AT mode. HW-748 is sold as a SIM800L breakout/kit; `SIM800L` only sends raw AT commands and exposes the serial stream. It is not a full SMS/call/network implementation. Verify logic levels and power supply requirements.

## SD and DFPlayer
`SDCard` wraps the Arduino SD API and uses the board's SPI pins plus chip select. `DFPlayer` wraps DFRobotDFPlayerMini. Insert and prepare a compatible microSD card before playback; track order and file naming follow the DFPlayer documentation.

## OV7670
`SUI_OV7670.h` wraps the upstream Arduino_OV767X library. Define `SUI_ENABLE_OV7670` before including the umbrella header. Upstream examples document supported boards such as Nano 33 BLE, with large frame buffers and limited frame rates. It is not a generic AVR Uno camera driver.

## Voltage and safety
- Check each breakout revision and pinout.
- Use level shifting for 3.3V-only inputs when connected to 5V boards.
- Do not power motors, servos or SIM800L from GPIO pins.
- Relays may switch hazardous mains voltage. Use enclosed, correctly rated equipment and qualified help.
