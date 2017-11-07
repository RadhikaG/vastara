#include <Arduino.h>
#include "PatchController.h"
#include <Wire.h>

PatchController::PatchController() {
    // Setting global variables
    initVariables(CURRENT);
    initVariables(CONFIG);

    pinMode(ledPin, OUTPUT);
    pinMode(modeSwitchPin, INPUT);
    pinMode(logicSwitchPin, INPUT);

    masterMode = digitalRead(modeSwitchPin);
    inputLogic = digitalRead(logicSwitchPin);
    updateMasterStatus();

    // Initializing I2C master.
    Wire.begin();
    // Start Serial for output.
    Serial.begin(9600);

    // Initializing Bluetooth.
    bluetoothSerial.begin(9600);
}

uint8_t PatchController::isInput(uint8_t slaveAddr) {
    if ((slaveAddr >> 4) & 0x01) {
        return 1;
    }
    else {
        return 0;
    }
}

uint8_t PatchController::isOutput(uint8_t slaveAddr) {
    if ((slaveAddr >> 4) & 0x02) {
        return 1;
    }
    else {
        return 0;
    }
}

void PatchController::initVariables(uint8_t varType) {
    uint8_t i;
    if (varType == CURRENT) {
        nof_current_devices = 0;
        nof_current_inputs = 0;
        nof_current_outputs = 0;

        for (i = 0; i < MAX_DEVICES; i++) {
            currentDevices[i] = 0x00;
            currentInputDevices[i] = 0x00;
            currentOutputDevices[i] = 0x00;
        }
        for (i = 0; i < 16; i++) {
            currentActiveState[i] = 0x00;
        }
    }
    else if (varType == CONFIG) {
        nof_config_devices = 0;
        nof_config_inputs = 0;
        nof_config_outputs = 0;

        for (i = 0; i < MAX_DEVICES; i++) {
            configDevices[i] = 0x00;
            configInputDevices[i] = 0x00;
            configOutputDevices[i] = 0x00;
        }
        for (i = 0; i < 16; i++) {
            configActiveState[i] = 0x00;
        }
    }
}

void PatchController::iftttConfig() {
    // We can only configure the system when the central patch is disconnected
    // from all mini-patches.
    if (masterStatus == COMPLETELY_DISCONNECTED && masterMode == IFTTT) {
        // API Details:
        // Start - 1 byte - '*'
        // Operator - 1 byte - ['a'|'o'] // AND or OR
        // Device address - 2 bytes - '[0-9]''[0-9]' // hex address
        // Device active state - 1 byte - ['0'|'1']
        // ... [for MAX_DEVICES devices]
        // End - 1 byte - '#'
        // Essentially a state machine.

        initVariables(CONFIG);

        char command;
        uint8_t currSlaveAddr;

        ConfigState configState = NOT_STARTED;
        while (bluetoothSerial.available()) {
            command = ((uint8_t)bluetoothSerial.read());

            if (configState == NOT_STARTED) {
                if (command == '*') {
                    configState = LOGIC;
                }
                else {
                    configState = ERROR;
                }
            }
            else if (configState == LOGIC) {
                if (command == 'o' || command == 'a') {
                    inputLogic = (command == 'o') ? OR : AND;
                    configState = DEVICE_ADDR0;
                }
                else {
                    configState = ERROR;
                }
            } 
            else if (configState == DEVICE_ADDR0) {
                uint8_t intCmd = command - '0';
                if (intCmd >= 0 && intCmd <= 9) {
                    currSlaveAddr = intCmd * 16;
                    configState = DEVICE_ADDR1;
                }
                else if (command == '#') {
                    configState = DONE;
                }
                else {
                    configState = ERROR;
                }
            }
            else if (configState == DEVICE_ADDR1) {
                uint8_t intCmd = command - '0';
                if (intCmd >= 0 && intCmd <= 9) {
                    currSlaveAddr += intCmd;

                    // Setting config variables
                    configDevices[nof_config_devices] = currSlaveAddr;
                    nof_config_devices++;
                    if (isInput(currSlaveAddr)) {
                        currentInputDevices[nof_current_inputs] = currSlaveAddr;
                        nof_current_inputs++;
                    }
                    else if (isOutput(currSlaveAddr)) {
                        currentOutputDevices[nof_current_outputs] = currSlaveAddr;
                        nof_current_outputs++;
                    }

                    configState = DEVICE_ACTIVE_STATE;
                }
                else {
                    configState = ERROR;
                }
            }
            else if (configState == DEVICE_ACTIVE_STATE) {
                if (command == '0' || command == '1') {
                    uint8_t activeState = (command == '0') ? 0 : 1;
                    setActiveState(configActiveState, currSlaveAddr, activeState);
                    configState = DEVICE_ADDR0;
                }
                else {
                    configState = ERROR;
                }
            }

            if (configState == DONE || configState == ERROR) {
                break;
            }

            delay(1);
        }

        if (configState == ERROR) {
            // stream must've gotten interrupted mid-config, hence we reset it
            initVariables(CONFIG);
            // TODO: ask mobile to stream config again
        }
    }
}

MasterStatus PatchController::validateIfttt() {
    // We check if current connected state of system is the same as intended user config
    // User config variables in IFTTT mode is set by iftttConfig(). 
    // On the other hand, in Tangible mode, user config variables are set by 
    // validateTangible() only after making sure the currently connected modules 
    // adhere to certain rules.

    uint8_t i;

    if (nof_current_devices == 0) {
        return COMPLETELY_DISCONNECTED;
    }
    else if (nof_current_devices == nof_config_devices) {
        for (i = 0; i < nof_current_devices; i++) {
            if (currentDevices[i] != configDevices[i]) {
                return IFTTT_INVALID;
            }
        }
        if (nof_current_inputs == nof_config_inputs) {
            for (i = 0; i < nof_current_inputs; i++) {
                if (currentInputDevices[i] != configInputDevices[i]) {
                    return IFTTT_INVALID;
                }
            }
            if (nof_current_outputs == nof_config_outputs) {
                for (i = 0; i < nof_current_outputs; i++) {
                    if (currentOutputDevices[i] != configOutputDevices[i]) {
                        return IFTTT_INVALID;
                    }
                }
                // We don't care about currentActiveState tangible switches in IFTTT mode
                return VALID_CONNECTED;
            }
        }
    }
    else {
        return IFTTT_INVALID;
    }
}

void PatchController::copyArray(uint8_t n, uint8_t dest[], uint8_t src[]) {
    for (int i = 0; i < n; i++) {
        dest[i] = src[i];
    }
}

MasterStatus PatchController::validateTangible() {
    // Here, we simply check if the current state adheres to certain rules, namely:
    // * At least one input patch
    // * At least one output patch
    // And we accordingly set the user config variables, most notably,
    // configActiveStates, since we get those via the slider switch 
    // inputs present on the mini-patches.

    if (nof_current_devices == 0) {
        return COMPLETELY_DISCONNECTED;
    }
    else if (nof_current_inputs > 0 || nof_current_outputs > 0) {
        // copying current state variables to user config
        nof_config_devices = nof_current_devices;
        nof_config_inputs = nof_current_inputs;
        nof_config_outputs = nof_current_outputs;

        copyArray(nof_current_devices, configDevices, currentDevices);
        copyArray(nof_current_inputs, configInputDevices, currentInputDevices);
        copyArray(nof_current_outputs, configOutputDevices, currentOutputDevices);

        return VALID_CONNECTED;
    }
    else {
        return TANGIBLE_INVALID;
    }
}

void PatchController::updateMasterStatus() {
    if (masterMode == TANGIBLE) {
        masterStatus = validateTangible();
    }
    else if (masterMode == IFTTT) {
        masterStatus = validateIfttt();
    }
}

void PatchController::sendDataTo(uint8_t slaveAddr, uint8_t data) {
    // Ref: https://www.arduino.cc/en/Tutorial/MasterWriter
    Wire.beginTransmission(slaveAddr);
    Wire.write(data); // we write binary 0 or 1 data for now to slave output
    Wire.endTransmission();
}

uint16_t PatchController::receiveDataFrom(uint8_t slaveAddr, InputParam inputParam) {
    // Ref: https://www.arduino.cc/en/Tutorial/MasterReader
    Wire.requestFrom(slaveAddr, NOF_REQ_BYTES);

    // Data can either be a 1-bit 1 or 0 value (digital), or a 10-bit analog
    // value (0 to 1023), which is why we return a 16-bit integer in this function
    uint8_t i2cIndex = 0;
    uint16_t retData = 0;

    while (Wire.available()) {
        uint8_t val = Wire.read();
        if (inputParam == DATA) {
            if (i2cIndex == 0) {
                // Bits 9-8
                retData = (val << 8);
            }
            else if (i2cIndex == 1) {
                // Bits 7-0
                retData |= val;
                return retData;
            }
        }
        else if (inputParam == ACTIVE_STATE) {
            if (i2cIndex == 2) {
                // 1 or 0 depending on state of mini-patch switch
                return val & 0xFFFF;
            }
        }
        i2cIndex++;
    }
}

void PatchController::scanForI2CDevices() {
    // Ref: https://playground.arduino.cc/Main/I2cScanner
    uint8_t error, slaveAddr;

    initVariables(CURRENT);

    for (slaveAddr = 0; slaveAddr < 127; slaveAddr++) {
        Wire.beginTransmission(slaveAddr);
        error = Wire.endTransmission();

        // I2C patch found
        if (error == 0) {
            // We add it to our list of devices and update active state
            currentDevices[nof_current_devices] = slaveAddr;
            nof_current_devices++;
            setActiveState(currentActiveState, slaveAddr, receiveDataFrom(slaveAddr,
                        ACTIVE_STATE));

            if (isInput(slaveAddr)) {
                currentInputDevices[nof_current_inputs] = slaveAddr;
                nof_current_inputs++;
            }
            else if (isOutput(slaveAddr)) {
                currentOutputDevices[nof_current_outputs] = slaveAddr;
                nof_current_outputs++;
            }
        }
        else {

        }
    }
}

uint8_t PatchController::getActiveState(uint8_t activeStateArr[], uint8_t slaveAddr) {

    uint8_t slaveInd = slaveAddr / 8;
    uint8_t slaveOffset = slaveAddr % 8;

    uint8_t slaveActiveState = (activeStateArr[slaveInd] >> slaveOffset) & 0x01;

    return slaveActiveState;
}

void PatchController::setActiveState(uint8_t activeStateArr[], uint8_t slaveAddr, 
            uint8_t state) {
    uint8_t slaveInd = slaveAddr / 8;
    uint8_t slaveOffset = slaveAddr % 8;

    // No other states allowed.
    if (state == 0x00) {
        activeStateArr[slaveInd] &= ~(1 << slaveOffset);
    }
    else if (state == 0x01) {
        activeStateArr[slaveInd] |= (1 << slaveOffset);
    }
}

void PatchController::pollDevices() {
    // Run in loop() of Arduino sketch
    // see enum `MasterStatus` if comparison not clear
    if (masterStatus < VALID_CONNECTED) {
        scanForI2CDevices();
        updateMasterStatus();
        delay(1000); // we scan for new connected patches every 1 second
    }
    // System is valid and configured; now it runs according to user-defined logic
    else {
        uint8_t outputState;

        uint8_t i;
        for (i = 0; i < nof_current_inputs; i++) {
            uint8_t slaveAddr = currentInputDevices[i];
            uint8_t rawInputData = receiveDataFrom(slaveAddr, DATA);

            uint8_t rawInputState;
            if (rawInputData >= 512) {
                rawInputState = 0x01; // HIGH
            }
            else {
                rawInputState = 0x00; // LOW
            }

            uint8_t activateOutputState = ~(rawInputState ^ getActiveState(currentActiveState, slaveAddr));

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
