#include "PatchModule.h"

PatchModule::PatchModule(uint8_t slaveAddr) {
    I2C_SLAVE_ADDRESS = slaveAddr;

    // Dummy initialization values
    i2c_regs[0] = 0xDE;
    i2c_regs[1] = 0xAD;
    i2c_regs[2] = 0xBE;
    i2c_regs[3] = 0xEF;
    reg_position = 0;
}
