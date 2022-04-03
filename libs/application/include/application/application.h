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

template <typename T> void ApplicationEntry();


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

   public:
    static void bootstrap(void (*task_entry)(void*));

   protected:
    graphics::Display* getDisplay();
    uint32_t getUpdateCounter() const;
    void setPeriod(uint32_t period_ms);
    uint32_t getPeriod() const;

    uint32_t getMillis() const;
    uint32_t getDeltaMillis() const;
    void sleep(uint32_t millis) const;

    float getTime() const;
    float getDelta() const;

   protected:
    uint32_t getAvgCycleTime() const;
    uint32_t getAvgUpdatesPerSecond() const;
    void showStatistics(bool show);

   protected:
    void renderOverlay();

   private:
    bool running_{false};
    bool error_{false};
    uint32_t period_ms_{10};
    uint32_t timeofs_{0};
    uint32_t delta_time_ms_{0};
    uint32_t update_counter_{0};
    uint32_t avg_cycle_time_ms_{0};
    uint32_t avg_updates_per_sec_{0};
    bool show_stats_{false};
    graphics::Display* display_{nullptr};
    int exit_code_{0};

   public:
    _NODEFAULTS(Application);
};

}  // namespace application
