#include <Arduino.h>
#include <SoftwareSerial.h>
#include "PatchController.h"

PatchController::PatchController() {
    // Setting global variables
    nof_current_devices = 0;
    nof_current_inputs = 0;
    nof_current_outputs = 0;

    masterMode = digitalRead(modeSwitchPin);
    inputLogic = digitalRead(logicSwitchPin);
    masterStatus = updateMasterStatus();

    // Initializing I2C master.
    Wire.begin();
    // Start Serial for output.
    Serial.begin(9600);

    // Initializing Bluetooth.
    bluetoothSerial.begin(9600);
}

void PatchController::iftttConfig() {
    // We can only configure the system when the central patch is disconnected
    // from all mini-patches.
    if (masterStatus == COMPLETELY_DISCONNECTED && isIfttt) {
        if (bluetoothSerial.available()) {
            // TODO: fill in bluetooth API details for config
        }
    }
}

int PatchController::validateIfttt() {
    // We check if current connected state of system is the same as intended user config

}

int PatchController::validateTangible() {
    // Here, we simply check if the connected state adheres to certain rules, namely:
    // * At least one input patch
    // * At least one output patch
    // And we accordingly set the user config variables, most notably,
    // the activeStates of the devices, since we get those via the slider switch 
    // inputs present on the mini-patches.

}

void PatchController::updateMasterStatus() {
    if (masterMode == TANGIBLE) {
        if (validateTangible() == 1) {
            masterStatus = VALID_CONNECTED;
        }
        else if (validateTangible() == 0) {
            masterStatus = COMPLETELY_DISCONNECTED;
        }
        else if (validateTangible() == -1) {
            masterStatus = TANGIBLE_INVALID;
        }
    }
    else if (masterMode == IFTTT) {
        if (validateIfttt() == 1) {
            masterStatus = VALID_CONNECTED;
        }
        else if (validateIfttt() == 0) {
            masterStatus = COMPLETELY_DISCONNECTED;
        }
        else if (validateIfttt() == -1) {
            masterStatus = IFTTT_INVALID;
        }
    }
}

void PatchController::sendDataTo(uint8_t slaveAddr, uint8_t data) {
    // Ref: https://www.arduino.cc/en/Tutorial/MasterWriter
    Wire.beginTransmission(slaveAddr);
    Wire.write(data); // TODO: fill in data according to slave
    Wire.endTransmission();
}

void PatchController::receiveDataFrom(uint8_t slaveAddr) {
    // Ref: https://www.arduino.cc/en/Tutorial/MasterReader
    
    Wire.request(slaveAddr, NOF_REQ_BYTES);

    while (Wire.available()) {
        uint8_t data = Wire.read();
    }
}

void PatchController::scanForI2CDevices() {
    // Ref: https://playground.arduino.cc/Main/I2cScanner
    uint8_t error, addr;

    for (addr = 0; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        error = Wire.endTransmission();

        // I2C patch found
        if (error == 0) {
            
        }
        else {

        }
    }
}

void PatchController::getActiveState(uint8_t slaveAddr) {
    // TODO: bit magic on deviceActiveState
}

void PatchController::setActiveState(uint8_t slaveAddr, bool state) {
    // TODO: bit magic on deviceActiveState
}

void PatchController::pollDevices() {
    // Run in loop() of Arduino sketch
    // see enum `PatchStatus` if comparison not clear
    if (masterStatus < VALID_CONNECTED) {
        scanForI2CDevices();
        updateMasterStatus();
        delay(500); // we scan for new connected patches every 500 ms
    }
    // System is valid and configured; now it runs according to user-defined logic
    else {
        bool outputState;

        uint8_t i;
        for (i = 0; i < nof_current_inputs; i++) {
            uint8_t slaveAddr = currentInputDevices[i];
            uint8_t rawInputData = receiveDataFrom(slaveAddr);
            uint8_t activateOutputState = ~(rawInputData ^ getActiveState(slaveAddr));

            // We perform no logical operations on the first input checked
            if (i == 0) {
                outputState = activateOutputState;
            }
            else {
                if (inputLogic == OR) {
                    outputState |= activateOutputState;
                }
                else if (inputLogic == AND) {
                    outputState &= activateOutputState;
                }
            }
        }

        for (i = 0; i < nof_current_outputs; i++) {
            uint8_t slaveAddr = currentOutputDevices[i];
            sendDataTo(slaveAddr, outputState);
        }

        delay(100);
    }
}
