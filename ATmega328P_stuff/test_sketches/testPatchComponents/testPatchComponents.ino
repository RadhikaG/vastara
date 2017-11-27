#include <SoftwareSerial.h>

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

static const uint8_t ssRX = A0; ///< SoftwareSerial
static const uint8_t ssTX = A1; ///< SoftwareSerial
SoftwareSerial bluetoothSerial(ssRX, ssTX);

uint8_t color;

void setup() {
    pinMode(statusLEDRed, OUTPUT);
    pinMode(statusLEDBlue, OUTPUT);
    pinMode(statusLEDGreen, OUTPUT);
    pinMode(statusLEDCommon, OUTPUT);

    pinMode(modeSwitchPin, INPUT);
    pinMode(logicSwitchPin, INPUT);
    pinMode(iftttConfigButton, INPUT);

    color = 0;

    Serial.begin(9600);
    bluetoothSerial.begin(9600);
}

void loop() {
    Serial.println(digitalRead(modeSwitchPin));
    Serial.println(digitalRead(logicSwitchPin));
    Serial.println(digitalRead(iftttConfigButton));

    if (color == 0) {
        digitalWrite(statusLEDRed, HIGH);
        digitalWrite(statusLEDGreen, LOW);
        digitalWrite(statusLEDBlue, LOW);
    }
    else if (color == 1) {
        digitalWrite(statusLEDRed, LOW);
        digitalWrite(statusLEDGreen, HIGH);
        digitalWrite(statusLEDBlue, LOW);
    }
    else if (color == 2) {
        digitalWrite(statusLEDRed, LOW);
        digitalWrite(statusLEDGreen, LOW);
        digitalWrite(statusLEDBlue, HIGH);
    }

    if (bluetoothSerial.available()) {
        Serial.write(bluetoothSerial.read());
    }

    Serial.println("----------------Done------------");

    color = (color + 1) % 3;
    delay(1000);
}
