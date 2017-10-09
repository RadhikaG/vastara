#include <Arduino.h>
#include "AlcoholModule.h"

AlcoholModule::AlcoholModule() : InputPatchModule(ALC_SLAVE_ADDR) {
    pinMode(alcoholPin, INPUT);
}

void AlcoholModule::readSensor() {
    uint16_t alcoholVal = analogRead(alcoholPin);
    i2c_regs[0] = alcoholVal >> 8; // Bits 9-8 of `lightVal`
    i2c_regs[1] = alcoholVal & 0xFF; // Bits 7-0 of `lightVal`
    i2c_regs[2] = digitalRead(switchPin) & 0xFF; // 0 or 1 depending on state of switch
}
