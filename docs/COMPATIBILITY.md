# Compatibility and wiring notes

## HW-504 joystick
Common KY-023 style module: VRx and VRy are analog outputs; SW is a momentary push switch. Use SUI::Joystick(xPin, yPin, switchPin).

## HW-040 rotary encoder
Common KY-040 style module: CLK, DT, SW. Use SUI::RotaryEncoder(clk, dt, sw) and call update() frequently from loop().

## Stepper motors
Stepper4Wire drives four logic inputs in a basic sequence. Connect through a proper driver such as ULN2003 or another compatible driver. It is blocking and has no acceleration, current control, stall detection or microstepping. A4988/DRV8825-style STEP/DIR drivers need a different adapter.

## Seven-segment and DIP switches
The direct seven-segment class handles one digit with segment order a,b,c,d,e,f,g,dp. Put a current-limiting resistor in series with every segment. Common-anode and common-cathode wiring must match the constructor flag. Multiplexed displays and TM1637/MAX7219 modules need controller-specific support. DIP switches use INPUT_PULLUP; ON means the switch connects the pin to GND.

## Microphones and beepers
AnalogMicrophone reads an amplified analog output (AO). SoundDetector reads a digital comparator output (DO). A bare electret capsule needs bias and a preamplifier; it is not directly supported as a bare capsule. Passive buzzers use tone(); active buzzers use digital on/off.

## HC-05 / HC-06 Bluetooth
BluetoothUART wraps a Stream; pass Serial1 or a SoftwareSerial instance. Pairing, baud rate and AT mode are not configured automatically. Check UART voltage levels; use a divider/level shifter on module RX when needed.

## HW-748 / SIM800L
HW-748 is listed by some sellers as a SIM800L GSM/GPRS module. SUI::SIM800L is only a low-level AT-command UART helper, not a full SMS/call/data stack. SIM800L requires a stable supply with high peak current and appropriate antenna/SIM setup. Check the exact board variant and logic levels.

## SD / microSD
SUI_SD.h wraps Arduino SD. Use a proper 3.3 V level-shifted microSD board with 5 V MCUs. Chip-select and SPI pins depend on the board.

## DFPlayer Mini
SUI_DFPlayer.h wraps DFRobotDFPlayerMini. Use a suitable serial port, correctly formatted microSD card and speaker wiring recommended by the module documentation.

## LCD / TFT / servo / camera
- I2C character LCD: HD44780 + common PCF8574 backpack mapping only.
- TFT: Adafruit_GFX-compatible controller libraries; the adapter is not itself a controller driver.
- Servo: requires Arduino Servo library and a suitable external supply for larger servos.
- OV7670 is not implemented yet; capture support is MCU- and board-specific.

## Safety
Never power motors, servos, SIM800L or high-current loads from GPIO pins. Relays may switch hazardous mains voltage: use enclosed, correctly rated equipment and qualified help.
