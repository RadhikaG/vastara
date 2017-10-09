#ifndef ALCOHOL_H
#define ALCOHOL_H

#include <Arduino.h>
#include "InputPatchModule.h"
#define ALC_SLAVE_ADDR 0x13

class AlcoholModule : InputPatchModule {

public:

    static const uint8_t alcoholPin = A3;

    AlcoholModule();
    void readSensor();
};

#endif
