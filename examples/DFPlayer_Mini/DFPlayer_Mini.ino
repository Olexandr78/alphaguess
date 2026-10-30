#define SUI_ENABLE_DFPLAYER
#include <SashaUniversalIO.h>
#include <SoftwareSerial.h>

SoftwareSerial mp3Serial(10, 11); // Arduino RX, TX; cross-connect to DFPlayer TX, RX
SUI::DFPlayer player;

void setup() {
  Serial.begin(9600);
  mp3Serial.begin(9600);
  if (!player.begin(mp3Serial)) {
    Serial.println("DFPlayer not found");
    while (true) {}
  }
  player.volume(18);
  player.play(1);
}

void loop() {}
