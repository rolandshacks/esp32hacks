//
// Application
//
#pragma once

#include "app/bits.h"

class Application {

    public:
        explicit Application();
        void run();

    public:
        virtual void loop();
        virtual void stop();
        virtual bool is_running() const;
        virtual bool has_error() const;

    public:
        virtual void init();
        virtual void update(uint32_t frame_counter, uint32_t cycle_counter);

    private:
        static void task_entry(void* pvParameters);

    protected:
        uint32_t get_time() const;
        float get_timef() const;
        Display* get_display();
        void sleep(uint32_t millis) const;

    private:
        bool running_;
        bool error_;
        uint32_t timeofs_;
        Display* display_;

    public:
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;
        ~Application() = default;
};
