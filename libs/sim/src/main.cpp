//
// Simulator entry
//

#include "sim/sim.h"

bool app_update() { return simulator::Sim::instance()->update(); }

int main(int argc, const char* argv[])
{
    simulator::Sim sim;
    sim.init();
    return 0;
}
