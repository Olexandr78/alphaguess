#include <SashaUniversalIO.h>

// Pin map for the selected board. ESP32 ADC inputs use GPIO34/35;
// encoder inputs use GPIO18/19/21 and relay output uses GPIO22.
#if defined(ARDUINO_ARCH_ESP32)
constexpr uint8_t JOY_X = 34;
constexpr uint8_t JOY_Y = 35;
constexpr uint8_t JOY_SW = 23;
constexpr uint8_t ENC_CLK = 18;
constexpr uint8_t ENC_DT = 19;
constexpr uint8_t ENC_SW = 21;
constexpr uint8_t RELAY_PIN = 22;
#else
constexpr uint8_t JOY_X = A0;
constexpr uint8_t JOY_Y = A1;
constexpr uint8_t JOY_SW = 2;
constexpr uint8_t ENC_CLK = 3;
constexpr uint8_t ENC_DT = 4;
constexpr uint8_t ENC_SW = 5;
constexpr uint8_t RELAY_PIN = 7;
#endif

// HW-504 joystick
SUI::Joystick joystick(JOY_X, JOY_Y, JOY_SW);
// HW-040 rotary encoder
SUI::RotaryEncoder encoder(ENC_CLK, ENC_DT, ENC_SW);
SUI::Relay relay(RELAY_PIN, SUI::ActiveLevel::LOW_LEVEL);

void setup() {
  Serial.begin(9600);
  joystick.begin(); encoder.begin(); relay.begin();
}
void loop() {
  encoder.update();
  Serial.print("X="); Serial.print(joystick.x());
  Serial.print(" Y="); Serial.print(joystick.y());
  Serial.print(" encoder="); Serial.println(encoder.position());
  if (joystick.pressed()) relay.toggle();
  delay(5);
}
