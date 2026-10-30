#pragma once
#include <Arduino.h>
#include <Arduino_OV767X.h>

// Optional adapter. Install Arduino_OV767X separately. The upstream driver
// currently targets specific supported boards (not every Arduino MCU).
namespace SUI {
class OV7670Camera {
public:
  bool begin(int resolution=QVGA,int format=RGB565,int fps=1,int cameraModel=OV7670) {
    _ready = (Camera.begin(resolution,format,fps,cameraModel) == 1);
    return _ready;
  }
  void end(){ if(_ready) Camera.end(); _ready=false; }
  bool ready() const{return _ready;}
  int width() const{return _ready?Camera.width():0;}
  int height() const{return _ready?Camera.height():0;}
  int bytesPerPixel() const{return _ready?Camera.bytesPerPixel():0;}
  size_t frameSize() const{return (size_t)width()*(size_t)height()*(size_t)bytesPerPixel();}
  bool capture(void *buffer){ if(!_ready || !buffer)return false; Camera.readFrame(buffer); return true; }
  void testPattern(int pattern=2){ if(_ready) Camera.testPattern(pattern); }
private:
  bool _ready=false;
};
}
