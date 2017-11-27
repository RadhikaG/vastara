#include <TinyWireS.h>
#include "InputPatchModule.h"

LDRModule ldr;

void requestEvent() {
    ldr.requestEvent();
}

void receiveEvent(int numBytes) {
    ldr.receiveEvent(numBytes);
}

void setup() {
    TinyWireS.begin(LDR_SLAVE_ADDR);
    TinyWireS.onRequest(requestEvent);
    TinyWireS.onReceive(receiveEvent);
}

void loop() {
    TinyWireS_stop_check();
}
