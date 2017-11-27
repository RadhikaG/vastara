#include <Wire.h>
#define SLAVE_ADDR 0x17
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
    Wire.beginTransmission(slaveAddr);
    Wire.write(2);
    Wire.endTransmission();

    Serial.println("Set reg");

//    delay(2000);
//    
//    Wire.requestFrom(slaveAddr, 1);
//
//    Serial.println("Available: ");
//    Serial.println(Wire.available());
//    
//    if (Wire.available() != 1) {
//        Serial.println("Nothing here.");
//    }
//
//    uint8_t val = Wire.read();
//    Serial.println(val, HEX);
//    Serial.println("-----------Done------------");
}


void loop() {
    int switchVal = receiveDataFrom(SLAVE_ADDR, 0);
    //Serial.println(switchVal);
  
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);

    delay(1000); // we request data every 1 sec
}
