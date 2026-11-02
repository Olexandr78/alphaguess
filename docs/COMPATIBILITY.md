# Compatibility, setup and wiring

## HW-504 joystick
VRx and VRy connect to analog inputs; SW is a momentary switch. `SUI::Joystick(xPin,yPin,swPin)` uses INPUT_PULLUP for SW.

## HW-040 rotary encoder
CLK, DT and optional SW connect to digital inputs. `SUI::RotaryEncoder(clk,dt,sw)` uses quadrature transition decoding; call `update()` frequently. Mechanical bounce and wiring can still affect readings.

## Generic inputs and switches
`AnalogInput` reads analog outputs; `DigitalInput` reads digital outputs with configurable active polarity and pull-up. `DebouncedButton` provides non-blocking debounce events.

## Stepper motors
- `Stepper4Wire` is a basic blocking 8-phase output sequence for a suitable four-input driver board (e.g. ULN2003).
- `StepDirStepper` emits STEP/DIR pulses for A4988, DRV8825 or compatible drivers. Configure current limit, microstep pins and pulse timing. No acceleration planning or stall detection is implemented.

Never connect a motor winding directly to MCU pins.

## Displays
- `SevenSegment` controls one bare digit directly; order is a,b,c,d,e,f,g,dp. Use one current-limiting resistor per segment.
- `TM1637Display` handles 4-digit TM1637 boards.
- `MAX7219Display` handles MAX7219/MAX7221 serial segment modules.
- `I2CLCD` targets HD44780 LCDs with a common PCF8574 backpack mapping.
- `ParallelLCD` drives HD44780 in 4-bit parallel mode with RW tied to GND.

## Microphones and beepers
The analog microphone helper reads an already amplified analog output. A bare electret capsule requires bias and a preamplifier. Passive piezo buzzers use `tone()`.

## Common sensors
Optional wrappers support DHT11/DHT22, DS18B20 and BMP280 using their established third-party libraries. Define the corresponding `SUI_ENABLE_...` macro before including the main header.

## Bluetooth and GSM
`BluetoothUART` wraps any Arduino `Stream`; `SIM800L` sends raw AT commands and is not a full SMS/call/network stack. Verify logic levels and supply requirements.

## SD and DFPlayer
`SDCard` wraps Arduino SD and uses board SPI pins plus chip select. `DFPlayer` wraps DFRobotDFPlayerMini.

## OV7670 (ESP32-WROOM-32)

Define `SUI_ENABLE_OV7670` before including `SashaUniversalIO.h`. The native implementation:
- Generates XCLK on the selected GPIO using ESP32 LEDC (default 10 MHz; configurable from 1 to 24 MHz).
- Starts XCLK before SCCB access and checks sensor PID 0x76.
- Configures QQVGA RGB565 (160x120) and receives pixel data through the original ESP32 I2S0 camera-slave mode with linked DMA descriptors.
- Exposes `captureFrame(dst, dstSize, timeoutMs)`; success returns 38,400 bytes. The example prints a checksum and initial bytes.
- Accepts a custom GPIO map for every signal. Constructor order is SCL, SDA, VS, HS/HREF, PCLK (module PLK), XCLK (module XLK), D7..D0, RET, PWDN.
- Preserves `RET` as labelled and never drives it because its function on this board revision is unconfirmed.
- Does not call `Wire.end()` from camera `end()`, because the I2C bus may be shared.

The backend targets the original ESP32 only. It uses low-level ESP32 I2S/GPIO/DMA registers and does not use `esp_camera` or another camera library. Capture timing, signal polarity and RGB565 byte order depend on sensor/module configuration; verify actual image output on the target wiring. A successful compile or SCCB PID read alone does not prove a valid frame. The example is `examples/OV7670_Capture/OV7670_Capture.ino`.

## Voltage and safety
- Check each breakout revision and pinout.
- ESP32 GPIO is 3.3V-only and not 5V tolerant.
- Motors, servos and SIM800L need suitable external supplies/drivers, never GPIO power.
- Relays may switch hazardous mains voltage. Use enclosed, correctly rated equipment and qualified help.
