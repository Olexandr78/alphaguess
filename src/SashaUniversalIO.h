#pragma once

// Main entry point. Core classes and common pin-based modules are available
// from this single include. Optional adapters that depend on third-party
// libraries are included explicitly in their own headers.
#include <Arduino.h>
#include <Wire.h>
#include "SUI_Modules.h"

// Core: Relay, Button, Buzzer, HCSR04.
// Additional modules: Joystick (HW-504), RotaryEncoder (HW-040),
// Stepper4Wire, SevenSegment, DipSwitch, AnalogMicrophone,
// SoundDetector, ActiveBeeper, BluetoothUART (HC-05/HC-06), SIM800L (HW-748).
