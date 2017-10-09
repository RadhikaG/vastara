#ifndef LDR_H
#define LDR_H

#include <Arduino.h>
#include "InputPatchModule.h"
#define I2C_SLAVE_ADDRESS 0x11

class LDRModule : public InputPatchModule {

public:

    static const uint8_t LDRPin = A3;

    LDRModule();
    void readSensor();
}

#endif
