//
// ESP32 System
//

#include "sim/sim.h"

void esp_restart(void) {
    auto sim = simulator::Sim::instance();
    if (nullptr != sim) {
        sim->restart();
    }
}
