#ifndef ALCOHOL_H
#define ALCOHOL_H

#include <Arduino.h>
#include "InputPatchModule.h"
#define I2C_SLAVE_ADDRESS 0x13

class AlcoholModule : InputPatchModule {

public:

    static const uint8_t AlcoholPin = A3;

    AlcoholModule();
    void readSensor();
}

#endif
