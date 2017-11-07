#include "PatchController.h"

PatchController masterPatch;

void setup() {

}

void loop() {
    masterPatch.pollDevices();
}
