#include <Arduino.h>
#include <TinyWireS.h>

#include "OutputPatchModule.h"

OutputPatchModule::OutputPatchModule(uint8_t slaveAddr) : 
    PatchModule(slaveAddr) {}

void OutputPatchModule::receiveEvent(int numBytes) {
    digitalWrite(LEDPin, HIGH);
    delayMicroseconds(100000); // 0.1 sec
    digitalWrite(LEDPin, LOW);

    // Some sanity checks...
    if (numBytes < 1) {
        return;
    }
    if (numBytes > TWI_RX_BUFFER_SIZE) {
        return;
    }

    //reg_position = TinyWireS.receive();

    //numBytes--;

    //if (numBytes == 0) {
    //    // write was only to set buffer position `reg_position` for next read
    //    return;
    //}

    //while (numBytes > 0) {
    //    i2c_regs[reg_position] = TinyWireS.receive();
    //    reg_position++;
    //    if (reg_position >= reg_size) {
    //        reg_position = 0;
    //    }

    //    numBytes--;
    //}

    // The master patch always sends 1 byte, which is the output state
    if (numBytes == 1) {
        i2c_regs[0] = TinyWireS.receive();
        writeOutput();

        return;
    }
}

VibMotorModule::VibMotorModule() : OutputPatchModule(VIB_SLAVE_ADDR) {
    pinMode(vibMotorPin, OUTPUT);
}

void VibMotorModule::writeOutput() {
    uint8_t state = i2c_regs[0];
    // writes HIGH or LOW depending on state being 1 or 0 resp.
    digitalWrite(vibMotorPin, state);
}

RGBModule::RGBModule() : OutputPatchModule(RGB_SLAVE_ADDR) {
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
}

void RGBModule::writeOutput() {
    uint8_t color = i2c_regs[0];
    if (color == 0) {
        digitalWrite(redPin, LOW);
        digitalWrite(greenPin, LOW);
        digitalWrite(bluePin, LOW);
    }
    if (color == 1) {
        digitalWrite(redPin, HIGH);
        digitalWrite(greenPin, LOW);
        digitalWrite(bluePin, LOW);
    }
    else if (color == 2) {
        digitalWrite(redPin, LOW);
        digitalWrite(greenPin, HIGH);
        digitalWrite(bluePin, LOW);
    }
    else if (color == 3) {
        digitalWrite(redPin, LOW);
        digitalWrite(greenPin, LOW);
        digitalWrite(bluePin, HIGH);
    }
}
