#ifndef INPUTMODULE_H
#define INPUTMODULE_H

#include "PatchModule.h"
#include "PatchAddress.h"

class InputPatchModule : public PatchModule {

public:

    static const uint8_t switchPin = 4; // tangible switch

    InputPatchModule(uint8_t slaveAddr);

    // transfer data from `i2c_regs` of mini-patch to central patch; 
    // common protocol for all input patches
    void requestEvent();

    // The receive event for input patches is only to
    // set `reg_position` for the next read.
    void receiveEvent(int numBytes);

    // Unique for each type of sensor, ie. derived classes of `InputPatchModule`; 
    // reads sensors and decides how to fill up `i2c_regs`.
    //
    // General form of `i2c_regs` is:
    // - For analog sensors
    // Byte 0: Bits 9-8 of 10-bit analog value read
    // Byte 1: Bits 7-0 of 10-bit analog value read
    // Byte 2: Integer value of tangible switch (0/1 currently)
    // - For digital sensors
    // Byte 0: 0
    // Byte 1: 0 or 1 (digital value read)
    // Byte 2: Integer value of tangible switch (0/1 currently)
    //
    // `readSensor()` runs in the background (not in the main loop) using timer 
    // interrupts
    virtual void readSensor() = 0;
};


class AlcoholModule : InputPatchModule {

public:

    static const uint8_t alcoholPin = A3;

    AlcoholModule();
    void readSensor();
};

class LDRModule : public InputPatchModule {

public:

    static const uint8_t LDRPin = A3;

    LDRModule();
    void readSensor();
};

class FlexModule : public InputPatchModule {

public:

    static const uint8_t flexPin = A3;

    FlexModule();
    void readSensor();
};

class ButtonModule : public InputPatchModule {

public:

    static const uint8_t buttonPin = 3;

    ButtonModule();
    void readSensor();
};

#endif
