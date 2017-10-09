#ifndef VIBMOTOR_H
#define VIBMOTOR_H

#include <Arduino.h>
#include "OutputPatchModule.h"
#define VIB_SLAVE_ADDR 0x21

class VibMotorModule : public OutputPatchModule {

public:

    static const uint8_t vibMotorPin = 3; 

    VibMotorModule();
    void writeOutput();
};

#endif
