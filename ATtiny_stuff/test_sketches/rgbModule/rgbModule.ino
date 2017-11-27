#include <TinyWireS.h>
#include "OutputPatchModule.h"

RGBModule rgb;

void receiveEvent(uint8_t howMany) {
    rgb.receiveEvent(howMany);
}

void setup() {
    TinyWireS.begin(RGB_SLAVE_ADDR);
    TinyWireS.onReceive(receiveEvent);
}

void loop() {
    TinyWireS_stop_check();
}
