#pragma once
#include <Arduino.h>
#include <Wire.h>

#define ES8388_ADDR 0x10

class ES8388 {
  public:
    bool begin(TwoWire &w = Wire, uint8_t addr = ES8388_ADDR);
    void setOutputVolume(uint8_t vol);  // 0–100
    void mute(bool m);
    void outputEnable(bool en);
  private:
    TwoWire *_wire;
    uint8_t _addr;
    void writeReg(uint8_t reg, uint8_t val);
};