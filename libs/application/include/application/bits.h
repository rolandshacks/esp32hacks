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

#define DECLARE_APP(APP) \
    void app_bootstrap();                                   \
                                                            \
    void user_main(void*) {                                 \
        APP app;                                            \
        app.taskRun();                                     \
    }                                                       \
                                                            \
    extern "C" void app_main(void) {                        \
        app_bootstrap();                                    \
    }
