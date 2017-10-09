#include <Arduino.h>
#include <TinyWireS.h>

#include "OutputPatchModule.h"

OutputPatchModule::OutputPatchModule() : PatchModule() {}

void OutputPatchModule::receiveEvent(int numBytes) {
    // Some sanity checks...
    if (numBytes < 1) {
        return;
    }
    if (numBytes > TWI_RX_BUFFER_SIZE) {
        return;
    }

    reg_position = TinyWireS.receive();

    numBytes--;

    if (numBytes == 0) {
        // Write was only to set buffer for next read
        return;
    }

    while (numBytes > 0) {
        i2c_regs[reg_position] = TinyWireS.receive();
        reg_position++;
        if (reg_position >= reg_size) {
            reg_position = 0;
        }
    }
}
