#ifndef INPUTMODULE_H
#define INPUTMODULE_H

#include "PatchModule.h"

class InputPatchModule : public PatchModule {

public:

    static const uint8_t switchPin = 4; // tangible switch 

    void requestEvent(); // transfer data from `i2c_regs` of mini-patch to central patch; common protocol for all input patches
    virtual void readSensor() = 0; // unique for each type of sensor, ie. derived classes of `InputPatchModule`; decides how to fill up `i2c_regs`
}

#endif
