#ifndef OUTPUTMODULE_H
#define OUTPUTMODULE_H

#include "PatchModule.h"
#define TWI_RX_BUFFER_SIZE 16

class OutputPatchModule : public PatchModule {

public:

    OutputPatchModule(uint8_t slaveAddr);

    void receiveEvent(int numBytes); // transfer data from central patch to fill up `i2c_regs`; common protocol for all output patches
    virtual void writeOutput() = 0; // unique for each type of output module, ie. derived classes of `OutputPatchModule`; decides what to do with data in `i2c_regs`
};

#endif
