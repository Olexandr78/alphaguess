#pragma once
#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

namespace SUI {
class DFPlayer {
public:
  bool begin(Stream &serial, bool isACK=false, bool doReset=true) {
    return _player.begin(serial, isACK, doReset);
  }
  void volume(uint8_t value) { _player.volume(value > 30 ? 30 : value); }
  void play(uint16_t track) { _player.play(track); }
  void playFolder(uint8_t folder, uint8_t track) { _player.playFolder(folder, track); }
  void pause() { _player.pause(); }
  void resume() { _player.start(); }
  void stop() { _player.stop(); }
  void next() { _player.next(); }
  void previous() { _player.previous(); }
  void equalizer(uint8_t mode) { _player.EQ(mode); }
  DFRobotDFPlayerMini &driver() { return _player; }
private:
  DFRobotDFPlayerMini _player;
};
}
