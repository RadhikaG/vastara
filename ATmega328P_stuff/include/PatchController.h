#ifndef PATCHCONTROLLER_H
#define PATCHCONTROLLER_H

#include <SoftwareSerial.h>
#include <inttypes.h>
#define MAX_DEVICES 5 ///< 128 is theoretical limit for I2C; this is for our system
#define CURRENT 0
#define CONFIG 1

//---------- Global ISR Routines and Variables -------------//

enum MasterState {
    COMPLETELY_DISCONNECTED,
    IFTTT_IN_PROGRESS,
    INVALID,
    VALID_CONNECTED
}; 

enum MasterMode {
    TANGIBLE,
    IFTTT
};

enum InputLogic {
    OR,
    AND
};

enum SlaveParam {
    DATA = 0,
    ACTIVE_STATE = 2
};

enum IFTTTConfigState {
    NOT_STARTED,
    LOGIC,
    DEVICE_ADDR0,
    DEVICE_ADDR1,
    DEVICE_ACTIVE_STATE,
    DONE,
    ERROR
};


//-----------------------------------------------------------------------//

/* \brief Central Patch <-> Patch Module <-> Sensor/Device
 *
 * uC: ATmega328P
 */
class PatchController {

public:
    //**************** PatchController Pins *********************//

    // Digital pins for HC-05 bluetooth module. We don't use the default
    // built-in UART interface for Bluetooth, instead we're using SoftwareSerial 
    // on 2 digital pins, to use the default UART for PC debugging.
    static const uint8_t ssRX = A0; ///< SoftwareSerial
    static const uint8_t ssTX = A1; ///< SoftwareSerial

    // RGB LED for status LED; common cathode
    static const uint8_t statusLEDRed = 6; ///< PWM
    static const uint8_t statusLEDGreen = 9; ///< PWM
    static const uint8_t statusLEDBlue = 10; ///< PWM
    static const uint8_t statusLEDCommon = 7; ///< any digital pin for Timer1

    // Digital input pin for slider switch to switch between tangible and IFTTT
    // mode.
    static const uint8_t modeSwitchPin = 2; ///< External interrupt

    // Digital input pin for slider switch to switch between AND and OR for 
    // input combining.
    static const uint8_t logicSwitchPin = 3; ///< External interrupt

    // Push-button for initializing IFTTT config transfer process
    static const uint8_t iftttConfigButton = 8;

    // Analog-in 4 and 5 reserved for SDA and SCL for I2C respectively.
    static const uint8_t sdaPin = A4;
    static const uint8_t sclPin = A5;

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

    uint8_t nof_config_devices;
    uint8_t configDevices[MAX_DEVICES];
    uint8_t nof_config_inputs;
    uint8_t configInputDevices[MAX_DEVICES];
    uint8_t nof_config_outputs;
    uint8_t configOutputDevices[MAX_DEVICES];
    uint8_t configActiveState[16]; // 16 * 8 = 128

    //**************** Class functions *********************//

    PatchController();

    //------------ Interrupt functions ---------------------//

    static void statusLEDISR();
    static void masterModeSwitchISR();
    static void inputLogicSwitchISR();

    static void setStatusLED();
    void initInterrupts();

    //-----------------------------------------------------//

    // Takes in integer stream from mobile app via Bluetooth, and configures
    // the application logic accordingly.
    void iftttConfig();

    // Validates connected devices for operation, then sets logic depending on
    // tangible or IFTTT.
    // Called by validateMasterState().
    MasterState validateIfttt();
    MasterState validateTangible();
    void validateMasterState();

    // I2C-specific functions for dumping data to and fro.
    void sendDataTo(uint8_t slaveAddr, uint8_t data);
    int receiveDataFrom(uint8_t slaveAddr, SlaveParam slaveParam);

    // Finds all the I2C devices on the bus. 
    // Invokes validateIfttt or validateTangible after scanning done.
    void scanForI2CDevices();

    // Application logic loop.
    void pollDevices();

    //------------------ Helper functions ------------------//

    static void setStatusLEDColor(uint8_t red, uint8_t green, uint8_t blue);
    uint8_t isInput(uint8_t slaveAddr);
    uint8_t isOutput(uint8_t slaveAddr);
    void initVariables(uint8_t varType);
    void copyArray(uint8_t n, uint8_t dest[], uint8_t src[]);

    uint8_t getActiveState(uint8_t activeStateArr[], uint8_t slaveAddr);
    void setActiveState(uint8_t activeStateArr[], uint8_t slaveAddr, uint8_t state);

    //------------------------------------------------------//
};

#endif
