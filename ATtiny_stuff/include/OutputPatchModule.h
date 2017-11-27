#ifndef OUTPUTMODULE_H
#define OUTPUTMODULE_H

#include "PatchModule.h"
#include "PatchAddress.h"

class OutputPatchModule : public PatchModule {

public:

    OutputPatchModule(uint8_t slaveAddr);

    // transfer data from central patch to fill up `i2c_regs`; 
    // common protocol for all output patches
    void receiveEvent(int numBytes); 

    // unique for each type of output module, ie. derived classes of 
    //`OutputPatchModule`; decides what to do with data in `i2c_regs`
    virtual void writeOutput() = 0; 
};

class VibMotorModule : public OutputPatchModule {

public:

    static const uint8_t vibMotorPin = 1; ///< PWM pin

    VibMotorModule();
    void writeOutput();
};

class RGBModule : public OutputPatchModule {

public:

    static const uint8_t redPin = 1; ///< Digital pin
    static const uint8_t greenPin = 3; ///< Digital pin
    static const uint8_t bluePin = 4; ///< Digital pin

    RGBModule();
    void writeOutput();
};

#endif
