#ifndef PATCHCONTROLLER_H
#define PATCHCONTROLLER_H

class PatchController {
    // Central Patch
    // Central Patch <-> Patch Module <-> Sensor/Device
    // Mux between the central patch and the set of patch
    // modules because all the patch modules have the same
    // I2C address.
    // uC: ATmega328P

public:

    PatchController();

}

#endif
