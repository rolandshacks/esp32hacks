//
// OLED Demo
//

#include "app/application.h"

#include <cmath>

class Hello : public Application {

public:
    explicit Hello() : Application() {}

    void init() {
        LOG_INFO("app", "Hello, world!");
    }

    void update(uint32_t frame_counter, uint32_t cycle_counter) {
        LOG_INFO("app", "Hello, world! (%d)", frame_counter);
    }

    _NOCOPY(Hello)
};

DECLARE_APP(Hello);
