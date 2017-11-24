#include <Wire.h>
#define SLAVE_ADDR 0x11
#define NOF_BYTES 4
#define DATA 0
#define SWITCH 1

void setup() {
    Wire.begin(); // pin 4 is SDA, pin 5 is SCL
    Serial.begin(9600);
    pinMode(13, OUTPUT);
}


int receiveDataFrom(uint8_t slaveAddr, uint8_t slaveParam) {
    // Ref: https://www.arduino.cc/en/Tutorial/MasterReader
    Wire.requestFrom(slaveAddr, 4);

    // We have 4 available registers of uint8_t in all mini-patches
    if (Wire.available() != 4) {
        return -1;
    }

    // Data can either be a 1-bit 1 or 0 value (digital), or a 10-bit analog
    // value (0 to 1023), which is why we return a 16-bit integer in this function
    uint8_t i2cIndex = 0;
    int retData = 0;

    while (Wire.available()) {
        uint8_t val = Wire.read();
        if (slaveParam == DATA) {
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
        else if (slaveParam == SWITCH) {
            if (i2cIndex == 2) {
                // 1 or 0 depending on state of mini-patch switch
                return val & 0x0001;
            }
        }
        i2cIndex++;
    }
}


void loop() {
    int switchVal = receiveDataFrom(SLAVE_ADDR, 0);
    Serial.println(switchVal);
  
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);

    delay(1000); // we request data every 1 sec
}
