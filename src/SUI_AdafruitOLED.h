#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace SUI {
class AdafruitOLED {
public:
  AdafruitOLED(uint8_t width,uint8_t height,int8_t resetPin=-1,TwoWire &wire=Wire)
    :_display(width,height,&wire,resetPin){}
  bool begin(uint8_t address=0x3C){return _display.begin(SSD1306_SWITCHCAPVCC,address);}
  void clear(uint16_t color=SSD1306_BLACK){_display.clearDisplay();_display.fillScreen(color);}
  void text(int16_t x,int16_t y,const char *value,uint16_t color=SSD1306_WHITE,uint8_t size=1){
    _display.setCursor(x,y);_display.setTextColor(color);_display.setTextSize(size);_display.print(value);
  }
  void pixel(int16_t x,int16_t y,uint16_t color=SSD1306_WHITE){_display.drawPixel(x,y,color);}
  void flush(){_display.display();}
  Adafruit_SSD1306 &driver(){return _display;}
private:Adafruit_SSD1306 _display;
};
}
