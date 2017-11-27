#include "PatchController.h"

PatchController * masterPatch;

void setup() {
    masterPatch = new PatchController();
}

void loop() {
    masterPatch->pollDevices();
}
