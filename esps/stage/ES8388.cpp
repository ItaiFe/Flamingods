#include "ES8388.h"

bool ES8388::begin(TwoWire &w, uint8_t addr) {
  _wire = &w;
  _addr = addr;
  _wire->begin(33, 32); // SDA, SCL for ESP32-A1S
  delay(50);

  // Reset
  writeReg(0x00, 0x80);
  delay(100);
  writeReg(0x00, 0x00);

  // Power up DAC, select HP outputs
  writeReg(0x02, 0xF3);
  writeReg(0x08, 0x00);
  writeReg(0x2B, 0x80);
  writeReg(0x2C, 0x80);

  mute(false);
  setOutputVolume(80);
  outputEnable(true);

  return true;
}

void ES8388::writeReg(uint8_t reg, uint8_t val) {
  _wire->beginTransmission(_addr);
  _wire->write(reg);
  _wire->write(val);
  _wire->endTransmission();
}

void ES8388::mute(bool m) {
  writeReg(0x26, m ? 0x04 : 0x00);
  writeReg(0x27, m ? 0x04 : 0x00);
}

void ES8388::setOutputVolume(uint8_t vol) {
  if (vol > 100) vol = 100;
  uint8_t v = map(vol, 0, 100, 0, 0x28); // approx 0–40 dB
  writeReg(0x2F, v);
  writeReg(0x30, v);
}

void ES8388::outputEnable(bool en) {
  writeReg(0x2F, en ? 0x00 : 0x80);
  writeReg(0x30, en ? 0x00 : 0x80);
}