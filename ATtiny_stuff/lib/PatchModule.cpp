#include "PatchModule.h"

PatchModule::PatchModule() {
    i2c_regs[0] = 0xDE;
    i2c_regs[1] = 0xAD;
    reg_position = 0;
}
