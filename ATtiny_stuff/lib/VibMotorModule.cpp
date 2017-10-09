#include <Arduino.h>
#include "VibMotorModule.h"

VibMotorModule::VibMotorModule() : OutputPatchModule(VIB_SLAVE_ADDR) {
    pinMode(vibMotorPin, OUTPUT);
}

void VibMotorModule::writeOutput() {
    uint8_t state = i2c_regs[0];
    digitalWrite(vibMotorPin, state); // writes HIGH or LOW depending on state being 1 or 0 resp.
}
