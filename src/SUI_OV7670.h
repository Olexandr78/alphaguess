#pragma once

#include <Arduino.h>
#include <Wire.h>

#if defined(ARDUINO_ARCH_ESP32)
  #include <esp_arduino_version.h>
  #include <driver/gpio.h>
  #include <driver/periph_ctrl.h>
  #include <esp_heap_caps.h>
  #include <rom/lldesc.h>
  #include <soc/gpio_sig_map.h>
  #include <soc/i2s_struct.h>
#endif

// OV7670 control + original ESP32 I2S0 camera-slave/DMA receiver.
// No esp_camera, Arduino_OV767X or third-party camera driver is used.
namespace SUI {

struct OV7670Pins {
  int8_t vcc = -1;   // Documentation only: connect to 3.3V.
  int8_t dgnd = -1;  // Documentation only: connect to GND.
  int8_t scl = -1;
  int8_t sda = -1;
  int8_t vs = -1;
  int8_t hs = -1;   // OV7670 HREF / module label HS
  int8_t pclk = -1; // Module label PLK
  int8_t xclk = -1; // Module label XLK
  int8_t d7 = -1; int8_t d6 = -1; int8_t d5 = -1; int8_t d4 = -1;
  int8_t d3 = -1; int8_t d2 = -1; int8_t d1 = -1; int8_t d0 = -1;
  int8_t ret = -1;  // Exact board label RET; function not confirmed, never toggled.
  int8_t pwdn = -1;
};

class OV7670Camera {
public:
  static constexpr uint16_t FRAME_WIDTH = 160;
  static constexpr uint16_t FRAME_HEIGHT = 120;
  static constexpr size_t FRAME_BYTES = (size_t)FRAME_WIDTH * FRAME_HEIGHT * 2;

  // Order: SCL, SDA, VS, HS/HREF, PCLK(PLK), XCLK(XLK), D7..D0, RET, PWDN.
  OV7670Camera(int8_t scl, int8_t sda, int8_t vs, int8_t hs,
               int8_t pclk, int8_t xclk,
               int8_t d7, int8_t d6, int8_t d5, int8_t d4,
               int8_t d3, int8_t d2, int8_t d1, int8_t d0,
               int8_t ret = -1, int8_t pwdn = -1,
               TwoWire &wire = Wire, uint8_t address = 0x21)
      : _pins(), _wire(wire), _address(address) {
    _pins.scl=scl; _pins.sda=sda; _pins.vs=vs; _pins.hs=hs;
    _pins.pclk=pclk; _pins.xclk=xclk;
    _pins.d7=d7; _pins.d6=d6; _pins.d5=d5; _pins.d4=d4;
    _pins.d3=d3; _pins.d2=d2; _pins.d1=d1; _pins.d0=d0;
    _pins.ret=ret; _pins.pwdn=pwdn;
  }

  explicit OV7670Camera(const OV7670Pins &pins, TwoWire &wire=Wire,
                        uint8_t address=0x21)
      : _pins(pins), _wire(wire), _address(address) {}

  bool setXclkFrequency(uint32_t hz) {
    if (_ready || _xclkStarted || hz < 1000000UL || hz > 24000000UL) return false;
    _xclkHz=hz; return true;
  }
  uint32_t xclkFrequency() const { return _xclkHz; }
  bool xclkRunning() const { return _xclkStarted; }
  bool ready() const { return _ready; }
  uint8_t productId() const { return _productId; }
  const OV7670Pins &pins() const { return _pins; }

  bool begin() {
    _ready=false;
    if (!validatePins()) return false;
    if (_pins.pwdn >= 0) {
      pinMode((uint8_t)_pins.pwdn, OUTPUT);
      digitalWrite((uint8_t)_pins.pwdn, LOW);
      delay(2);
    }
#if defined(ARDUINO_ARCH_ESP32)
    if (_pins.xclk < 0 || !startXclk()) return false;
    delay(10);
#endif
    bool wireOk;
    if (_pins.scl >= 0 && _pins.sda >= 0) {
#if defined(ARDUINO_ARCH_ESP32)
      wireOk=_wire.begin((int)_pins.sda,(int)_pins.scl);
#else
      _wire.begin(); wireOk=true;
#endif
    } else {
      _wire.begin(); wireOk=true;
    }
    if (!wireOk) return false;
    delay(2);
    uint8_t pid=0;
    if (!readRegister(0x0A,pid)) return false;
    _productId=pid;
    if (pid != 0x76) return false;
    if (!resetSensor()) return false;
    if (!configureQQVGA_RGB565()) return false;
    _ready=true;
    return true;
  }

  void end() {
#if defined(ARDUINO_ARCH_ESP32)
    stopI2S();
#endif
    _ready=false;
    stopXclk();
  }

  bool startXclk() {
#if defined(ARDUINO_ARCH_ESP32)
    if (_xclkStarted) return true;
    if (_pins.xclk < 0) return false;
    const uint8_t pin=(uint8_t)_pins.xclk;
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    if (!ledcAttach(pin,_xclkHz,1)) return false;
    if (!ledcWrite(pin,1)) { ledcDetach(pin); return false; }
#else
    if (ledcSetup(_xclkChannel,_xclkHz,1)<=0.0) return false;
    ledcAttachPin(pin,_xclkChannel);
    ledcWrite(_xclkChannel,1);
#endif
    _xclkStarted=true; return true;
#else
    return false;
#endif
  }

  void stopXclk() {
#if defined(ARDUINO_ARCH_ESP32)
    if (!_xclkStarted) return;
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcDetach((uint8_t)_pins.xclk);
#else
    ledcWrite(_xclkChannel,0);
    ledcDetachPin((uint8_t)_pins.xclk);
#endif
    _xclkStarted=false;
#endif
  }

  bool writeRegister(uint8_t reg,uint8_t value) {
    _wire.beginTransmission(_address);
    if (_wire.write(reg)!=1 || _wire.write(value)!=1) return false;
    return _wire.endTransmission()==0;
  }

  bool readRegister(uint8_t reg,uint8_t &value) {
    _wire.beginTransmission(_address);
    if (_wire.write(reg)!=1) return false;
    if (_wire.endTransmission(false)!=0) return false;
    if (_wire.requestFrom((int)_address,1)!=1) return false;
    int v=_wire.read(); if (v<0) return false;
    value=(uint8_t)v; return true;
  }

  bool resetSensor() {
    if (!writeRegister(0x12,0x80)) return false;
    delay(10);
    return true;
  }

  // Program QQVGA RGB565. The sensor outputs 160x120, two bytes per pixel.
  bool configureQQVGA_RGB565() {
    static const uint8_t regs[][2] = {
      {0x12,0x04}, // COM7: RGB
      {0x8C,0x00}, // RGB444 off
      {0x40,0xD0}, // COM15: RGB565, full output range
      {0x3A,0x04}, // TSLB: byte order
      {0x0C,0x04}, // COM3: scale enable
      {0x3E,0x19}, // COM14: manual scaling, PCLK divider
      {0x70,0x3A}, // SCALING_XSC
      {0x71,0x35}, // SCALING_YSC
      {0x72,0x11}, // SCALING_DCWCTR: QQVGA downsample
      {0x73,0xF1}, // SCALING_PCLK_DIV
      {0xA2,0x02}, // SCALING_PCLK_DELAY
      {0x15,0x00}, // COM10: normal PCLK/HREF/VSYNC polarity
      {0x11,0x01}  // CLKRC: input clock divider
    };
    for (size_t i=0;i<sizeof(regs)/sizeof(regs[0]);++i) {
      if (!writeRegister(regs[i][0],regs[i][1])) return false;
    }
    delay(30);
    return true;
  }

  // Capture one QQVGA RGB565 frame into caller memory.
  // Returns FRAME_BYTES on success, 0 on timeout/error.
  // dst must have at least FRAME_BYTES bytes. On ESP32-WROOM-32 the internal
  // DMA buffer is used and copied to dst, so dst need not be DMA-capable.
  size_t captureFrame(uint8_t *dst,size_t dstSize,uint32_t timeoutMs=1500) {
#if defined(ARDUINO_ARCH_ESP32)
    if (!_ready || !dst || dstSize<FRAME_BYTES || timeoutMs==0) return 0;
    if (!startI2S()) return 0;
    uint8_t *dma=(uint8_t*)heap_caps_malloc(FRAME_BYTES,MALLOC_CAP_DMA|MALLOC_CAP_INTERNAL);
    const size_t chunk=4092;
    const size_t count=(FRAME_BYTES+chunk-1)/chunk;
    lldesc_t *descs=(lldesc_t*)heap_caps_calloc(count,sizeof(lldesc_t),MALLOC_CAP_DMA|MALLOC_CAP_INTERNAL);
    if (!dma || !descs) {
      if (dma) heap_caps_free(dma);
      if (descs) heap_caps_free(descs);
      stopI2S(); return 0;
    }
    for (size_t i=0,off=0;i<count;++i) {
      size_t n=FRAME_BYTES-off; if(n>chunk)n=chunk;
      descs[i].size=n; descs[i].length=0; descs[i].buf=dma+off;
      descs[i].eof=(i==count-1); descs[i].sosf=0; descs[i].owner=1;
      descs[i].qe.stqe_next=(i+1<count)?&descs[i+1]:NULL;
      off+=n;
    }

    // Synchronize to the next VSYNC edge before arming DMA.
    const uint32_t syncStart=millis();
    while (digitalRead((uint8_t)_pins.vs)==LOW) {
      if ((uint32_t)(millis()-syncStart)>=timeoutMs) {
        heap_caps_free(descs); heap_caps_free(dma); stopI2S(); return 0;
      }
      delay(1);
    }
    while (digitalRead((uint8_t)_pins.vs)==HIGH) {
      if ((uint32_t)(millis()-syncStart)>=timeoutMs) {
        heap_caps_free(descs); heap_caps_free(dma); stopI2S(); return 0;
      }
      delay(1);
    }

    I2S0.int_clr.val=0xFFFFFFFF;
    I2S0.rx_eof_num=FRAME_BYTES;
    I2S0.in_link.addr=(uint32_t)descs;
    I2S0.in_link.start=1;
    I2S0.conf.rx_start=1;
    const uint32_t start=millis();
    while (!I2S0.int_raw.in_done && !I2S0.int_raw.in_err_eof &&
           !I2S0.int_raw.in_dscr_err && !I2S0.int_raw.in_dscr_empty) {
      if ((uint32_t)(millis()-start)>=timeoutMs) break;
      delay(1);
    }
    const bool ok=I2S0.int_raw.in_done && !I2S0.int_raw.in_err_eof &&
                  !I2S0.int_raw.in_dscr_err;
    I2S0.conf.rx_start=0;
    I2S0.in_link.stop=1;
    if (ok) memcpy(dst,dma,FRAME_BYTES);
    heap_caps_free(descs); heap_caps_free(dma);
    stopI2S();
    return ok?FRAME_BYTES:0;
#else
    (void)dst; (void)dstSize; (void)timeoutMs;
    return 0;
#endif
  }

private:
  OV7670Pins _pins;
  TwoWire &_wire;
  uint8_t _address;
  uint8_t _productId=0;
  uint32_t _xclkHz=10000000UL;
  bool _ready=false;
  bool _xclkStarted=false;
  bool _i2sStarted=false;
#if defined(ARDUINO_ARCH_ESP32) && (!defined(ESP_ARDUINO_VERSION_MAJOR) || ESP_ARDUINO_VERSION_MAJOR<3)
  uint8_t _xclkChannel=7;
#endif

  bool validatePins() const {
    const int8_t p[]={_pins.scl,_pins.sda,_pins.vs,_pins.hs,_pins.pclk,_pins.xclk,
      _pins.d7,_pins.d6,_pins.d5,_pins.d4,_pins.d3,_pins.d2,_pins.d1,_pins.d0};
    for(size_t i=0;i<sizeof(p)/sizeof(p[0]);++i) if(p[i]<0 || p[i]>39) return false;
    return true;
  }

#if defined(ARDUINO_ARCH_ESP32)
  bool startI2S() {
    if (_i2sStarted) return true;
    periph_module_enable(PERIPH_I2S0_MODULE);
    I2S0.conf.rx_start=0;
    I2S0.conf.rx_reset=1; I2S0.conf.rx_reset=0;
    I2S0.conf.rx_fifo_reset=1; I2S0.conf.rx_fifo_reset=0;
    I2S0.lc_conf.in_rst=1; I2S0.lc_conf.in_rst=0;
    I2S0.int_ena.val=0;
    I2S0.int_clr.val=0xFFFFFFFF;

    const int dataPins[8]={_pins.d0,_pins.d1,_pins.d2,_pins.d3,
                           _pins.d4,_pins.d5,_pins.d6,_pins.d7};
    const int dataSignals[8]={I2S0I_DATA_IN0_IDX,I2S0I_DATA_IN1_IDX,
      I2S0I_DATA_IN2_IDX,I2S0I_DATA_IN3_IDX,I2S0I_DATA_IN4_IDX,
      I2S0I_DATA_IN5_IDX,I2S0I_DATA_IN6_IDX,I2S0I_DATA_IN7_IDX};
    for(int i=0;i<8;++i) {
      pinMode(dataPins[i],INPUT);
      gpio_matrix_in((uint32_t)dataPins[i],dataSignals[i],false);
    }
    pinMode(_pins.pclk,INPUT); pinMode(_pins.vs,INPUT); pinMode(_pins.hs,INPUT);
    gpio_matrix_in((uint32_t)_pins.pclk,I2S0I_WS_IN_IDX,false);
    gpio_matrix_in((uint32_t)_pins.vs,I2S0I_V_SYNC_IDX,false);
    gpio_matrix_in((uint32_t)_pins.hs,I2S0I_H_ENABLE_IDX,false);
    gpio_matrix_in(0x38,I2S0I_H_SYNC_IDX,false);

    I2S0.conf.val=0;
    I2S0.conf.rx_slave_mod=1;
    I2S0.conf.rx_right_first=0;
    I2S0.conf.rx_msb_shift=0;
    I2S0.conf2.val=0;
    I2S0.conf2.lcd_en=1;
    I2S0.conf2.camera_en=1;
    I2S0.fifo_conf.val=0;
    I2S0.fifo_conf.dscr_en=1;
    I2S0.fifo_conf.rx_data_num=32;\n    I2S0.fifo_conf.rx_fifo_mod=1;
    I2S0.fifo_conf.rx_fifo_mod_force_en=1;
    I2S0.conf_chan.val=0;
    I2S0.conf_chan.rx_chan_mod=1;
    I2S0.sample_rate_conf.rx_bits_mod=8;
    I2S0.lc_conf.val=0;
    I2S0.lc_conf.check_owner=1;
    I2S0.lc_conf.indscr_burst_en=1;
    _i2sStarted=true;
    return true;
  }

  void stopI2S() {
    if (!_i2sStarted) return;
    I2S0.conf.rx_start=0;
    I2S0.in_link.stop=1;
    I2S0.conf2.camera_en=0;
    I2S0.conf2.lcd_en=0;
    I2S0.int_ena.val=0;
    I2S0.int_clr.val=0xFFFFFFFF;
    _i2sStarted=false;
  }
#endif
};

} // namespace SUI
