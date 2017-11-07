#include <Arduino.h>
#include <TinyWireS.h>

#include "InputPatchModule.h"

InputPatchModule::InputPatchModule(uint8_t slaveAddr) : 
        PatchModule(slaveAddr) {
    pinMode(switchPin, INPUT);
}

void InputPatchModule::requestEvent() {
    TinyWireS.send(i2c_regs[reg_position]);

    reg_position++;
    if (reg_position >= reg_size) {
        reg_position = 0;
    }
}

LDRModule::LDRModule() : InputPatchModule(LDR_SLAVE_ADDR) {
    pinMode(LDRPin, INPUT);
}

void LDRModule::readSensor() {
    uint16_t lightVal = analogRead(LDRPin);
    i2c_regs[0] = lightVal >> 8; // Bits 9-8 of `lightVal`
    i2c_regs[1] = lightVal & 0xFF; // Bits 7-0 of `lightVal`
    i2c_regs[2] = digitalRead(switchPin) & 0xFF; // 0 or 1 depending on state of switch
}

AlcoholModule::AlcoholModule() : InputPatchModule(ALC_SLAVE_ADDR) {
    pinMode(alcoholPin, INPUT);
}

void AlcoholModule::readSensor() {
    uint16_t alcoholVal = analogRead(alcoholPin);
    i2c_regs[0] = alcoholVal >> 8; // Bits 9-8 of `lightVal`
    i2c_regs[1] = alcoholVal & 0xFF; // Bits 7-0 of `lightVal`
    i2c_regs[2] = digitalRead(switchPin) & 0xFF; // 0 or 1 depending on state of switch
}
