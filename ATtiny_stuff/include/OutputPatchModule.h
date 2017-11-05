#ifndef OUTPUTMODULE_H
#define OUTPUTMODULE_H

#include "PatchModule.h"
#define TWI_RX_BUFFER_SIZE 16
#define VIB_SLAVE_ADDR 0x21

class OutputPatchModule : public PatchModule {

public:

    OutputPatchModule(uint8_t slaveAddr);

    // transfer data from central patch to fill up `i2c_regs`; 
    //common protocol for all output patches
    void receiveEvent(int numBytes); 

    // unique for each type of output module, ie. derived classes of 
    //`OutputPatchModule`; decides what to do with data in `i2c_regs`
    virtual void writeOutput() = 0; 
};


class VibMotorModule : public OutputPatchModule {

public:

    static const uint8_t vibMotorPin = 3; 

    VibMotorModule();
    void writeOutput();
};

#endif
