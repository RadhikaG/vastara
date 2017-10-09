#include <usiTwiSlave.h>
#include <TinyWireS.h>

#include "LDRModule.h"

LDRModule ldr;

void setup() {
  TinyWireS.begin(PatchModule::I2C_SLAVE_ADDRESS);
  TinyWireS.onRequest(ldr.requestEvent);
}

void loop() {
  TinyWireS_stop_check();
}
