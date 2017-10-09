#include <Arduino.h>
#include "LDRModule.h"

LDRModule::LDRModule() : InputPatchModule(LDR_SLAVE_ADDR) {
    pinMode(LDRPin, INPUT);
}

void LDRModule::readSensor() {
    uint16_t lightVal = analogRead(LDRPin);
    i2c_regs[0] = lightVal >> 8; // Bits 9-8 of `lightVal`
    i2c_regs[1] = lightVal & 0xFF; // Bits 7-0 of `lightVal`
    i2c_regs[2] = digitalRead(switchPin) & 0xFF; // 0 or 1 depending on state of switch
}
