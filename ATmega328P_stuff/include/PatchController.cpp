#include <Arduino.h>
#include <Wire.h>
#include "TimerOne.h"

#include "PatchController.h"
#include "PatchAddress.h"

//---------- Static ISR Routines and Variables -------------//

volatile MasterState masterState; ///< value depends on status of central patch

// Modified by an ISR set for an onboard slider switch.
// Controls if system is configured tangibly or via IFTTT.
volatile MasterMode masterMode;

// The below is only valid for tangible mode.
// Controls whether the inputs are combined using OR or AND.
volatile InputLogic inputLogic;

volatile uint8_t statusLEDState; ///< to save state of blinking LED

SoftwareSerial bluetoothSerial(PatchController::ssRX, PatchController::ssTX);

// Variables for debouncing IFTTT config push-button
bool lastIftttButtonState;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

void PatchController::setStatusLEDColor(uint8_t red, uint8_t green, uint8_t blue) {
    #ifdef COMMON_ANODE
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
    #endif
    analogWrite(statusLEDRed, red);
    analogWrite(statusLEDGreen, green);
    analogWrite(statusLEDBlue, blue);
}

void PatchController::setStatusLED() {
    switch (masterState) {
        case COMPLETELY_DISCONNECTED: // Solid green
            Timer1.detachInterrupt();
            digitalWrite(statusLEDCommon, 0);
            setStatusLEDColor(0, 255, 0);
            break;
        case IFTTT_IN_PROGRESS: // Blinking green at 500 ms interval
            Timer1.attachInterrupt(statusLEDISR, 500000);
            setStatusLEDColor(0, 255, 0);
            break;
        case INVALID: // Blinking red at 1 sec interval
            Timer1.attachInterrupt(statusLEDISR, 1000000);
            setStatusLEDColor(255, 0, 0);
            break;
        case VALID_CONNECTED: // Solid blue
            Timer1.detachInterrupt();
            digitalWrite(statusLEDCommon, 0);
            setStatusLEDColor(0, 0, 255);
            break;
        default:
            break;
    }
}

void PatchController::statusLEDISR() {
    statusLEDState ^= 1; // toggling state of LED
    digitalWrite(statusLEDCommon, statusLEDState);
}

void PatchController::masterModeSwitchISR() {
    masterMode = (digitalRead(modeSwitchPin) == 0) ? TANGIBLE : IFTTT;
}

void PatchController::inputLogicSwitchISR() {
    inputLogic = (digitalRead(logicSwitchPin) == 0) ? OR : AND;
}

void PatchController::initInterrupts() {
    // External event interrupts for slider switches
    attachInterrupt(digitalPinToInterrupt(modeSwitchPin), masterModeSwitchISR, 
            CHANGE);
    attachInterrupt(digitalPinToInterrupt(logicSwitchPin), inputLogicSwitchISR, 
            CHANGE);

    // Pin change interrupt for push-button config.
    // Ref: https://playground.arduino.cc/Main/PinChangeInterrupt
    // TODO: We're currently doing this using polling in pollDevices(), 
    // do with pin-change interrupt.

    // Timer interrupt for status LED blinking every 1 second
    Timer1.initialize(1000000);
    // We enable/disable the timer ISR every time the master state changes
    Timer1.detachInterrupt();
}

//--------------------------------------------------------//

PatchController::PatchController() {
    // Setting global variables
    initVariables(CURRENT);
    initVariables(CONFIG);

    pinMode(statusLEDRed, OUTPUT);
    pinMode(statusLEDBlue, OUTPUT);
    pinMode(statusLEDGreen, OUTPUT);
    pinMode(statusLEDCommon, OUTPUT);
    statusLEDState = 0;

    pinMode(modeSwitchPin, INPUT);
    pinMode(logicSwitchPin, INPUT);
    pinMode(iftttConfigButton, INPUT);

    masterMode = digitalRead(modeSwitchPin);
    inputLogic = digitalRead(logicSwitchPin);

    lastIftttButtonState = LOW;

    // Initializing I2C master on digital pins 4 and 5.
    Wire.begin();
    // Start Serial for PC output.
    Serial.begin(9600);

    // Initializing Bluetooth.
    bluetoothSerial.begin(9600);

    initInterrupts();

    // All initialization done, now we start our system.

    scanForI2CDevices();
    validateMasterState();
}

uint8_t PatchController::isInput(uint8_t slaveAddr) {
    return ((slaveAddr >> 4) & 0x01) ? 1 : 0;
}

uint8_t PatchController::isOutput(uint8_t slaveAddr) {
    return ((slaveAddr >> 4) & 0x02) ? 1 : 0;
}

void PatchController::initVariables(uint8_t varType) {
    uint8_t i;
    if (varType == CURRENT) {
        nof_current_devices = 0;
        nof_current_inputs = 0;
        nof_current_outputs = 0;

        for (i = 0; i < MAX_DEVICES; i++) {
            currentDevices[i] = 0;
            currentInputDevices[i] = 0;
            currentOutputDevices[i] = 0;
        }
        for (i = 0; i < 16; i++) {
            currentActiveState[i] = 0;
        }
    }
    else if (varType == CONFIG) {
        nof_config_devices = 0;
        nof_config_inputs = 0;
        nof_config_outputs = 0;

        for (i = 0; i < MAX_DEVICES; i++) {
            configDevices[i] = 0;
            configInputDevices[i] = 0;
            configOutputDevices[i] = 0;
        }
        for (i = 0; i < 16; i++) {
            configActiveState[i] = 0;
        }
    }
}

void PatchController::iftttConfig() {
    // We can only configure the system when the central patch is disconnected
    // from all mini-patches.
    if (masterState == IFTTT_IN_PROGRESS && masterMode == IFTTT) {
        // API Details:
        // Start - 1 byte - '*'
        // Operator - 1 byte - ['a'|'o'] // AND or OR
        // Device address - 2 bytes - '[0-9]''[0-9]' // hex address
        // Device active state - 1 byte - ['0'|'1']
        // ... [for MAX_DEVICES devices]
        // End - 1 byte - '#'
        //
        // We expect to receive device addresses in increasing order from the
        // Android app. 
        // 
        // Essentially a state machine.

        char command;
        uint8_t currSlaveAddr;

        IFTTTConfigState iftttConfigState = NOT_STARTED;

        // Stream must've gotten interrupted mid-config, hence we reset it,
        // and ask Android to sent config again.
        while (iftttConfigState != DONE) {
            initVariables(CONFIG);
            // We tell our Android phone to send us config stream.
            bluetoothSerial.print('*');

            // We timeout our bluetooth connection after 5 seconds
            unsigned long timeout_interval = 5000;
            unsigned long startMillis = millis();
            bool timeout = false;

            while (!bluetoothSerial.available()) {
                // we wait for bluetooth to respond
                if (millis() - startMillis >= timeout_interval) {
                    timeout = true;
                    break;
                }
            }
            if (timeout) {
                // Break connection; we must start config process manually 
                // again.
                Serial.println("Bluetooth connection timed out.");
                iftttConfigState = ERROR;
                setStatusLED();
                break;
            }

            // Received data from bluetooth, now we process config received.
            while (bluetoothSerial.available()) {
                command = ((uint8_t)bluetoothSerial.read());

                if (iftttConfigState == NOT_STARTED) {
                    if (command == '*') {
                        iftttConfigState = LOGIC;
                    }
                    else {
                        iftttConfigState = ERROR;
                    }
                }
                else if (iftttConfigState == LOGIC) {
                    if (command == 'o' || command == 'a') {
                        inputLogic = (command == 'o') ? OR : AND;
                        iftttConfigState = DEVICE_ADDR0;
                    }
                    else {
                        iftttConfigState = ERROR;
                    }
                } 
                else if (iftttConfigState == DEVICE_ADDR0) {
                    // signed 8-bit value
                    char intCmd = command - '0';
                    if (command == '#') {
                        iftttConfigState = DONE;
                    }
                    else if (intCmd >= 0 && intCmd <= 9) {
                        currSlaveAddr = intCmd * 16;
                        iftttConfigState = DEVICE_ADDR1;
                    }
                    else {
                        iftttConfigState = ERROR;
                    }
                }
                else if (iftttConfigState == DEVICE_ADDR1) {
                    // signed 8-bit value
                    char intCmd = command - '0';
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

                        iftttConfigState = DEVICE_ACTIVE_STATE;
                    }
                    else {
                        iftttConfigState = ERROR;
                    }
                }
                else if (iftttConfigState == DEVICE_ACTIVE_STATE) {
                    if (command == '0' || command == '1') {
                        uint8_t activeState = (command == '0') ? 0 : 1;
                        setActiveState(configActiveState, currSlaveAddr, activeState);
                        iftttConfigState = DEVICE_ADDR0;
                    }
                    else {
                        iftttConfigState = ERROR;
                    }
                }

                if (iftttConfigState == DONE || iftttConfigState == ERROR) {
                    // We don't need the rest of the data in the serial buffer.
                    // If we end in ERROR, we start bluetooth config transfer
                    // again.
                    break;
                }

                delay(1);
            }
        }
    }
}

MasterState PatchController::validateIfttt() {
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
                return INVALID;
            }
        }
        if (nof_current_inputs == nof_config_inputs) {
            for (i = 0; i < nof_current_inputs; i++) {
                if (currentInputDevices[i] != configInputDevices[i]) {
                    return INVALID;
                }
            }
            if (nof_current_outputs == nof_config_outputs) {
                for (i = 0; i < nof_current_outputs; i++) {
                    if (currentOutputDevices[i] != configOutputDevices[i]) {
                        return INVALID;
                    }
                }
                // We don't care about currentActiveState tangible switches in IFTTT mode
                return VALID_CONNECTED;
            }
        }
    }
    else {
        return INVALID;
    }
}

void PatchController::copyArray(uint8_t n, uint8_t dest[], uint8_t src[]) {
    for (int i = 0; i < n; i++) {
        dest[i] = src[i];
    }
}

MasterState PatchController::validateTangible() {
    // Here, we simply check if the current state adheres to certain rules, namely:
    // * At least one input patch
    // * At least one output patch
    // And we accordingly set the user config variables, most notably,
    // configActiveStates, since we get those via the slider switch 
    // inputs present on the mini-patches.

    if (nof_current_devices == 0) {
        return COMPLETELY_DISCONNECTED;
    }
    else if (nof_current_inputs > 0 && nof_current_outputs > 0) {
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
        return INVALID;
    }
}

void PatchController::validateMasterState() {
    if (masterMode == TANGIBLE) {
        masterState = validateTangible();
    }
    else if (masterMode == IFTTT) {
        masterState = validateIfttt();
    }
    setStatusLED();
}

void PatchController::sendDataTo(uint8_t slaveAddr, uint8_t data) {
    // Ref: https://www.arduino.cc/en/Tutorial/MasterWriter
    Wire.beginTransmission(slaveAddr);
    Wire.write(data); // we write binary 0 or 1 data for now to slave output
    uint8_t error = Wire.endTransmission();

    if (error != 0) {
        // Transfer error; slave connection may have been lost
        scanForI2CDevices();
        validateMasterState();
        return;
    }
}

int PatchController::receiveDataFrom(uint8_t slaveAddr, SlaveParam slaveParam) {
    // Ref: https://www.arduino.cc/en/Tutorial/MasterReader

    if (slaveParam == DATA) {
        // Setting position to read from
        Wire.beginTransmission(slaveAddr);
        Wire.write(0);
        Wire.endTransmission();
    }
    else if (slaveParam == ACTIVE_STATE) {
        // Setting position to read from
        Wire.beginTransmission(slaveAddr);
        Wire.write(2);
        Wire.endTransmission();
    }
    
    Wire.requestFrom(slaveAddr, 1);

    // We should have 1 byte available to read
    if (Wire.available() != 1) {
        scanForI2CDevices();
        validateMasterState();
        return -1;
    }

    // Data can either be a 1-bit 1 or 0 value (digital), or a 10-bit analog
    // value (0 to 1023), which is why we return a 16-bit integer in this 
    // function
    int retData = 0;

    uint8_t val = Wire.read();

    if (slaveParam == DATA) {
        // Bits 9-8 of input data
        retData = (val << 8);

        Wire.beginTransmission(slaveAddr);
        Wire.write(1);
        Wire.endTransmission();

        Wire.requestFrom(slaveAddr, 1);

        // We should have 1 byte available to read
        if (Wire.available() != 1) {
            scanForI2CDevices();
            validateMasterState();
            return -1;
        }

        val = Wire.read();
        // Bits 7-0 of input data
        retData |= val;
        // Returning 10-bit value
        return retData;
    }
    else if (slaveParam == ACTIVE_STATE) {
        // 1 or 0 depending on state of mini-patch switch
        return val & 0xFFFF;
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

            if (isInput(slaveAddr)) {
                currentInputDevices[nof_current_inputs] = slaveAddr;
                // Active state only relevant for input patches
                setActiveState(currentActiveState, slaveAddr, 
                        receiveDataFrom(slaveAddr, ACTIVE_STATE));
                nof_current_inputs++;
            }
            else if (isOutput(slaveAddr)) {
                currentOutputDevices[nof_current_outputs] = slaveAddr;
                // For output patches, we just set the active state to 1
                setActiveState(currentActiveState, slaveAddr, 1);
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
    if (state == 0) {
        activeStateArr[slaveInd] &= ~(1 << slaveOffset);
    }
    else if (state == 1) {
        activeStateArr[slaveInd] |= (1 << slaveOffset);
    }
}

void PatchController::pollDevices() {
    // Run in loop() of Arduino sketch

    //------ Debouncing IFTTT push-button for user config ------//

    // We poll this button because the function to be run on this
    // button being pushed, is involved and time-consuming, hence
    // poorly suited for running as an ISR. We could set a flag
    // and run the function in the main loop as well, but the
    // pin-change interrupt (we've run out of external interrupts)
    // is quite involved with debouncing.
    // TODO: Make pin-change interrupt for IFTTT push-button.
    if (masterMode == IFTTT && masterState != IFTTT_IN_PROGRESS) {
        Serial.println("IFTTT mode, checking for config upload mode");
        
        bool iffftButtonState = digitalRead(iftttConfigButton);
        if (iffftButtonState == HIGH) {
            Serial.println("Config activate button pressed.");
                masterState = IFTTT_IN_PROGRESS;
                setStatusLED();
                iftttConfig();
                lastDebounceTime = millis();

                // Fresh config uploaded, we now check the slaves.
                scanForI2CDevices();
                validateMasterState();
        }

        delay(1000);
    }

    //---------------------------------------------------------//

    else if (masterState == COMPLETELY_DISCONNECTED ||
           masterState == INVALID) {
        if (masterState == COMPLETELY_DISCONNECTED) {
            Serial.println("Completely disconnected");
        }
        else {
            Serial.println("Invalid");
        }
        
        scanForI2CDevices();
        validateMasterState();
        // we scan for new connected patches every 1 second
        delay(1000);
    }

    // System is valid and configured; now it runs according to user-defined logic
    else if (masterState == VALID_CONNECTED) {
        Serial.println("Valid and connected");
        
        uint8_t outputState;

        uint8_t i;
        for (i = 0; i < nof_current_inputs; i++) {
            uint8_t slaveAddr = currentInputDevices[i];
            int rawInputData = receiveDataFrom(slaveAddr, DATA);

            // if change in master state detected during data transfer
            if (masterState != VALID_CONNECTED) {
                break;
            }

            uint8_t rawInputState;
            int highBound;

            switch(slaveAddr) {
                case LDR_SLAVE_ADDR: highBound = 800; break;
                case BTN_SLAVE_ADDR: highBound = 0; break;
                default: break;
            }
            if (rawInputData > highBound) {
                rawInputState = 1; // HIGH
            }
            else {
                rawInputState = 0; // LOW
            }

            uint8_t activateOutputState = ~(rawInputState ^ 
                    getActiveState(currentActiveState, slaveAddr));

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
            // if change in master state detected during data transfer
            if (masterState != VALID_CONNECTED) {
                break;
            }
        }

        // We poll connected devices every 500 ms
        delay(500);
    }
}
