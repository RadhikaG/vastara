#ifndef PATCHCONTROLLER_H
#define PATCHCONTROLLER_H

#include <Wire.h>
#include <inttypes.h>
#define NOF_REQ_BYTES 4 ///< check PatchModule.h for size of `i2c_regs`
#define MAX_DEVICES 5 ///< 128 is theoretical limit for I2C; this is for our system

//---------- Global ISR Routines and Variables -------------//

enum PatchStatus {
    COMPLETELY_DISCONNECTED,
    IFTTT_INVALID,
    TANGIBLE_INVALID,
    VALID_CONNECTED
}; 

enum PatchMode {
    TANGIBLE,
    IFTTT
};

enum InputLogic {
    OR,
    AND
};

volatile PatchStatus masterStatus; ///< value depends on status of central patch
// Sets the delay for the status LED blinking frequency, whenever the device
// status is changed.
void setStatusLEDDelay();
// Just makes the LED blink according to the delay set above.
void statusLEDISR();

// Modified by an ISR set for an onboard slider switch.
// Controls if system is configured tangibly or via IFTTT.
volatile PatchMode masterMode; 
// And the slider switch ISR for modifying the above variable.
void modeSwitchISR();

// The below is only valid for tangible mode.
// Controls whether the inputs are combined using OR or AND.
volatile InputLogic inputLogic;
// And the slider switch ISR for modifying the above variable.
void logicSwitchISR();

SoftwareSerial bluetoothSerial(ssRX, ssTX);

//-----------------------------------------------------------------------//

/* \brief Central Patch <-> Patch Module <-> Sensor/Device
 *
 * uC: ATmega328P
 */
class PatchController {

public:
    // Analog-in 4 and 5 reserved for SDA and SCL for I2C respectively.

    // Digital pins 10 and 11 for HC-05 bluetooth module. We don't use the default
    // built-in UART interface for Bluetooth, instead we're using SoftwareSerial 
    // on pins 10 and 11, to use the default UART for PC debugging.
    static const uint8_t ssRX = 10;
    static const uint8_t ssTX = 11;

    // Digital output pin 13 for status LED.
    static const uint8_t ledPin = 13;

    // TODO: change switch pins depending on interrupts
    // Digital input pin 4 for slider switch to switch between tangible and IFTTT
    // mode.
    static const uint8_t modeSwitchPin = 4;

    // Digital input pin 5 for slider switch to switch between AND and OR for 
    // input combining.
    static const uint8_t logicSwitchPin = 5;

    //--------- Current state of the system ----------------//

    uint8_t nof_current_devices;
    uint8_t currentDevices[MAX_DEVICES]; ///< stores addresses of the connected patches

    uint8_t nof_current_inputs;
    uint8_t currentInputDevices[MAX_DEVICES];

    uint8_t nof_current_outputs;
    uint8_t currentOutputDevices[MAX_DEVICES];

    // Bitvector to store active HIGH or active LOW of input/output addresses
    uint8_t currentActiveState[16]; ///< for a maximum of 128 addresses

    //-------------- Intended user config ----------------//
    // May not be the same as the current state of the system!
    // (which is why we have the validateIfttt() and the validateTangible()
    // functions to check if the current state is the same as the user config)

    uint8_t nof_config_inputs;
    uint8_t configInputDevices[MAX_DEVICES];
    uint8_t nof_config_outputs;
    uint8_t configOutputDevices[MAX_DEVICES];
    uint8_t configActiveState[16];

    //-----------------------------------------------------------------------//

    PatchController();

    // Takes in integer stream from mobile app via Bluetooth, and configures
    // the application logic accordingly.
    void iftttConfig();
    // Validates connected devices for operation, then sets logic depending on
    // tangible or IFTTT.
    // Called by setStatus.
    void validateIfttt();
    void validateTangible();
    void setStatus();

    // I2C-specific functions for dumping data to and fro.
    void sendDataTo(uint8_t slaveAddr, uint8_t data);
    void receiveDataFrom(uint8_t slaveAddr);

    // Finds all the I2C devices on the bus. 
    // Invokes validateIfttt or validateTangible after scanning done.
    void scanForI2CDevices();
    // Application logic loop.
    void pollDevices();
}

#endif
