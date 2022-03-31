//
// Sim
//

#include "sim/sim.h"

bool app_update() { return simulator::Sim::instance()->update(); }

#ifdef SDL_main_h_
extern "C" int SDL_main(int argc, char *argv[])
#else
int main(int argc, const char* argv[])
#endif
{
    simulator::Sim sim;
    sim.init();
    return 0;
}
