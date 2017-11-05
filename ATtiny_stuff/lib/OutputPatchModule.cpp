#include <Arduino.h>
#include <TinyWireS.h>

#include "OutputPatchModule.h"

OutputPatchModule::OutputPatchModule(uint8_t slaveAddr) : 
    PatchModule(slaveAddr) {}

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
        // write was only to set buffer position `reg_position` for next read
        return;
    }

    while (numBytes > 0) {
        i2c_regs[reg_position] = TinyWireS.receive();
        reg_position++;
        if (reg_position >= reg_size) {
            reg_position = 0;
        }

        numBytes--;
    }
}

VibMotorModule::VibMotorModule() : OutputPatchModule(VIB_SLAVE_ADDR) {
    pinMode(vibMotorPin, OUTPUT);
}

void VibMotorModule::writeOutput() {
    uint8_t state = i2c_regs[0];
    digitalWrite(vibMotorPin, state); // writes HIGH or LOW depending on state being 1 or 0 resp.
}
