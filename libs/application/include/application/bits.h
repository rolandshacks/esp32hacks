//
// Includes
//

#pragma once

#include "esp_log.h"
#include "esp_system.h"

#define LOG_ERROR ESP_LOGE
#define LOG_WARN ESP_LOGW
#define LOG_INFO ESP_LOGI
#define LOG_DEBUG ESP_LOGD
#define LOG_VERBOSE ESP_LOGV

#define _NODEFAULTS(T) \
    public: \
    T(const T&) = delete; \
    T(const T&&) = delete; \
    T& operator=(const T&) = delete; \
    T& operator=(const T&&) = delete; \
    virtual ~T() = default;

#define DECLARE_APP_TYPE(APP) using __application_t = APP;

#define TASK_ENTRY_IMPL()                                                   \
    static void task_entry(void*) {                                         \
        __application_t app;                                                \
        app.taskRun();                                                      \
    }

#define BOOTSTRAP_IMPL()                                                    \
    template<> void application::ApplicationEntry<__application_t>() {      \
        __application_t::bootstrap(task_entry);                             \
    }

#ifndef SIMULATOR
    #define MAIN_IMPL()                                                     \
        extern "C" void app_main(void) {                                    \
            application::ApplicationEntry<__application_t>();               \
        }
#else
    #define MAIN_IMPL() /* */
#endif

#define DECLARE_APP(APP)                        \
    DECLARE_APP_TYPE(APP)                       \
    TASK_ENTRY_IMPL()                           \
    BOOTSTRAP_IMPL()                            \
    MAIN_IMPL()

#define DECLARE_SIM_ENTRY(APP)                  \
    class APP;                                  \
    void app_main() {                           \
        application::ApplicationEntry<APP>();   \
    }
