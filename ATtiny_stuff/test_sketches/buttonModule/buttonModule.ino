#include <TinyWireS.h>
#include "InputPatchModule.h"

ButtonModule btn;

void requestEvent() {
    btn.requestEvent();
}

void receiveEvent(uint8_t numBytes) {
    btn.receiveEvent(numBytes);
}

void setup() {
    TinyWireS.begin(BTN_SLAVE_ADDR);
    TinyWireS.onRequest(requestEvent);
    TinyWireS.onReceive(receiveEvent);
}

void loop() {
    TinyWireS_stop_check();
}
