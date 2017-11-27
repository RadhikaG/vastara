#include <Arduino.h>
#include <TinyWireS.h>

#include "InputPatchModule.h"

InputPatchModule::InputPatchModule(uint8_t slaveAddr) : 
        PatchModule(slaveAddr) {
    pinMode(switchPin, INPUT);
}

void InputPatchModule::receiveEvent(int numBytes) {
    // The receive event for input patches is only to
    // set `reg_position`.

    // Some sanity checks...
    
    if (numBytes < 1 || numBytes > TWI_RX_BUFFER_SIZE) {
                
        return;
    }
    // The master patch always sends 1 byte, which is the index of `i2c_regs`,
    // ie. `reg_position`
    else if (numBytes == 1) {
        reg_position = TinyWireS.receive();
        
        return;
    }
    else {
digitalWrite(LEDPin, HIGH);
        delayMicroseconds(100000); // 0.1 sec
        digitalWrite(LEDPin, LOW);
        return;
    }
}

void InputPatchModule::requestEvent() {
    readSensor();
    TinyWireS.send(i2c_regs[reg_position]);

    reg_position++;
    if (reg_position >= reg_size) {
        reg_position = 0;
    }
    
    digitalWrite(LEDPin, HIGH);
    delayMicroseconds(100000); // 0.1 sec
    digitalWrite(LEDPin, LOW);
}

LDRModule::LDRModule() : InputPatchModule(LDR_SLAVE_ADDR) {
    pinMode(LDRPin, INPUT);
}

void LDRModule::readSensor() {
    uint16_t lightVal = analogRead(LDRPin);
    i2c_regs[0] = lightVal >> 8; // Bits 9-8 of `lightVal`
    i2c_regs[1] = lightVal & 0xFF; // Bits 7-0 of `lightVal`
    i2c_regs[2] = digitalRead(switchPin) & 0xFF; // 0 or 1 depending on state of switch
}

AlcoholModule::AlcoholModule() : InputPatchModule(ALC_SLAVE_ADDR) {
    pinMode(alcoholPin, INPUT);
}

void AlcoholModule::readSensor() {
    uint16_t alcoholVal = analogRead(alcoholPin);
    i2c_regs[0] = alcoholVal >> 8; // Bits 9-8 of `lightVal`
    i2c_regs[1] = alcoholVal & 0xFF; // Bits 7-0 of `lightVal`
    i2c_regs[2] = digitalRead(switchPin) & 0xFF; // 0 or 1 depending on state of switch
}

FlexModule::FlexModule() : InputPatchModule(FLEX_SLAVE_ADDR) {
    pinMode(flexPin, INPUT);
}

void FlexModule::readSensor() {
    uint16_t flexVal = analogRead(flexPin);
    i2c_regs[0] = flexVal >> 8; // Bits 9-8 of `lightVal`
    i2c_regs[1] = flexVal & 0xFF; // Bits 7-0 of `lightVal`
    i2c_regs[2] = digitalRead(switchPin) & 0xFF; // 0 or 1 depending on state of switch
}

ButtonModule::ButtonModule() : InputPatchModule(BTN_SLAVE_ADDR) {
    pinMode(buttonPin, INPUT);
}

void ButtonModule::readSensor() {
    uint16_t buttonVal = digitalRead(buttonPin);
    i2c_regs[0] = 0;
    i2c_regs[1] = buttonVal & 0xFF; // Just digital value of the button 
    i2c_regs[2] = digitalRead(switchPin) & 0xFF; // 0 or 1 depending on state of switch
}
