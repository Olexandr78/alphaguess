#include <SashaUniversalIO.h>
#include <SUI_SD.h>

SUI::SDCard card;
void setup() {
  Serial.begin(9600);
  if (!card.begin(10)) { Serial.println("SD init failed"); return; }
  File file=card.open("/hello.txt", FILE_WRITE);
  if (file) { file.println("SashaUniversalIO SD test"); file.close(); }
  Serial.println(card.exists("/hello.txt") ? "File saved" : "No file");
}
void loop() {}
