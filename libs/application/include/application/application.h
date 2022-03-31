//
// Application
//
#pragma once

#include <cstdint>

#include "application/bits.h"

namespace graphics {
class Display;
}

namespace application {

/**
 * Application base
 */
class Application {

   public:
    explicit Application();

   public:
    virtual void init();
    virtual void update();

   public:
    virtual void taskRun();

   private:
    virtual void taskInit();
    virtual void taskLoop();

   public:
    virtual void exit(int exit_code);
    virtual bool isRunning() const;
    virtual bool hasError() const;

   protected:
    uint32_t get_time() const;
    float get_timef() const;
    graphics::Display* get_display();
    void sleep(uint32_t millis) const;
    uint32_t get_delta_ms() const;
    float get_delta_time() const;
    uint32_t get_update_counter() const;
    float get_update_frequency() const;
    uint32_t get_period_ms() const;
    void set_period_ms(uint32_t period);

   private:
    bool running_{false};
    bool error_{false};
    uint32_t period_ms_{10};
    uint32_t timeofs_{0};
    uint32_t delta_time_ms_{0};
    float update_frequency_{0.0f};
    uint32_t update_counter_{0};
    graphics::Display* display_{nullptr};
    int exit_code_{0};

   public:
    _NODEFAULTS(Application);
};

}  // namespace application
