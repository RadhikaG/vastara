#include <Wire.h>
#define SLAVE_ADDR 0x23

uint8_t color;

void setup() {
    color = 0;

    Wire.begin(); // pin 4 is SDA, pin 5 is SCL
    Serial.begin(9600);
    pinMode(13, OUTPUT);
}

int sendDataTo(uint8_t slaveAddr, uint8_t data) {
    // Ref: https://www.arduino.cc/en/Tutorial/MasterWriter
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(data);
    Wire.endTransmission();
    Serial.println(data);
    Serial.println("-----------Sent data------------");
}


void loop() {
    sendDataTo(SLAVE_ADDR, color);
    color = (color + 1) % 3; 
  
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);

    delay(1000); // we change LED color every 1 sec
}
