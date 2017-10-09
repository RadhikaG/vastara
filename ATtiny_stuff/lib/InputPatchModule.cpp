#include <Arduino.h>
#include <TinyWireS.h>

#include "InputPatchModule.h"

InputInputPatchModule(uint8_t slaveAddr) : 
    PatchModule(slaveAddr) {
    pinMode(switchPin, INPUT);
}

void InputrequestEvent() {
    TinyWireS.send(i2c_regs[reg_position]);

    reg_position++;
    if (reg_position >= reg_size) {
        reg_position = 0;
    }
}
