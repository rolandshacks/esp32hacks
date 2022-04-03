//
// OLED Demo
//

#include "application/application.h"

class Hello : public application::Application {
   public:
    explicit Hello() : Application() {}

    void init() override {
        LOG_INFO("app", "Hello, world!");
    }

    void update() override {
        LOG_INFO("app", "Hello, world! (%d)", getUpdateCounter());
    }

    _NODEFAULTS(Hello)
};

DECLARE_APP(Hello);
