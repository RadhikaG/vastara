#include <usiTwiSlave.h>
#include <TinyWireS.h>

#include "InputPatchModule.h"
#include "OutputPatchModule.h"

LDRModule ldr;

void requestEvent() {
    ldr.requestEvent();
}

void setup() {
    TinyWireS.begin(LDR_SLAVE_ADDR);
    TinyWireS.onRequest(requestEvent);
}

void loop() {
    TinyWireS_stop_check();
}
