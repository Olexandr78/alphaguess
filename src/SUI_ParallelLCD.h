#pragma once
#include <Arduino.h>

namespace SUI {
// HD44780-compatible character LCD in 4-bit parallel mode (RW tied to GND).
class ParallelLCD {
public:
  ParallelLCD(uint8_t rs,uint8_t enable,uint8_t d4,uint8_t d5,uint8_t d6,uint8_t d7,
              uint8_t columns=16,uint8_t rows=2)
    :_rs(rs),_en(enable),_data{d4,d5,d6,d7},_columns(columns),_rows(rows){}
  void begin(){
    pinMode(_rs,OUTPUT);pinMode(_en,OUTPUT);
    for(uint8_t i=0;i<4;i++)pinMode(_data[i],OUTPUT);
    delay(50);digitalWrite(_rs,LOW);
    write4(0x03);delay(5);write4(0x03);delayMicroseconds(150);write4(0x03);write4(0x02);
    command(0x28);command(0x0C);command(0x06);clear();
  }
  void clear(){command(0x01);delay(2);}
  void home(){command(0x02);delay(2);}
  void setCursor(uint8_t column,uint8_t row){
    static const uint8_t offsets[4]={0x00,0x40,0x14,0x54};
    if(row>=_rows)row=_rows-1;if(column>=_columns)column=_columns-1;
    command((uint8_t)(0x80|offsets[row]+column));
  }
  size_t write(uint8_t value){send(value,true);return 1;}
  void print(const char *s){if(!s)return;while(*s)write((uint8_t)*s++);}
private:
  uint8_t _rs,_en,_data[4],_columns,_rows;
  void pulse(){digitalWrite(_en,HIGH);delayMicroseconds(1);digitalWrite(_en,LOW);delayMicroseconds(50);}
  void write4(uint8_t n){for(uint8_t i=0;i<4;i++)digitalWrite(_data[i],(n>>i)&1?HIGH:LOW);pulse();}
  void send(uint8_t value,bool data){digitalWrite(_rs,data?HIGH:LOW);write4(value>>4);write4(value&0x0F);}
  void command(uint8_t value){send(value,false);}
};
}
