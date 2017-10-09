#include <Arduino.h>
#include <TinyWireS.h>

#include "InputPatchModule.h"

void InputPatchModule::requestEvent() {
    TinyWireS.send(i2c_regs[reg_position]);

    reg_position++;
    if (reg_position >= reg_size) {
        reg_position = 0;
    }
}
