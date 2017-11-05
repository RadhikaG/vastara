#ifndef PATCHMODULE_H
#define PATCHMODULE_H

#include <inttypes.h>

class PatchModule {
    // Abstract base class for patch modules
    // All patch modules inherit from this class;
    // can't be explicitly instantiated.
    // PatchModule is an ATtiny I2C slave connected
    // to a ATmega328P master. It provides a uniform
    // interface to any sensor/output module subclasses
    // for the ATmega328P to access data to and fro.
    //
    // Central Patch <-> Patch Module <-> Sensor/Device
    //
    // uC: ATtiny45/85
    //
    // pwrPin = pin 8 of IC
    // gndPin = pin 4 of IC
    //
    //                  +-\/-+
    // Ain0 (D 5) PB5  1|    |8  Vcc
    // Ain3 (D 3) PB3  2|    |7  PB2 (D 2) Ain1
    // Ain2 (D 4) PB4  3|    |6  PB1 (D 1) pwm1
    //            GND  4|    |5  PB0 (D 0) pwm0
    //                  +----+

public:

    // IC pin 5 and 7 are reserved for I2C SDA and SCL respectively.
    // IC pin 4 and 8 are GND and PWR resp.
    // Rest are available to patch module sensors/output modules.

    // for i2c
    uint8_t I2C_SLAVE_ADDRESS;
    static const uint8_t reg_size = 4;
    volatile uint8_t i2c_regs[reg_size];
    volatile uint8_t reg_position;

    // InputPatchModules essentially just fill up `i2c_regs` in different ways for each subclass of `InputPatchModule`.
    // OutputPatchModules read off `i2c_regs` in different ways for each subclass of `OutputPatchModule`.

    PatchModule(uint8_t slaveAddr);
};

#endif
