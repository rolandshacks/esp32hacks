#pragma once

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/gpio.h"

#include "oled/oled.h"
typedef oled::OLED Display;

#define _NOCOPY(T) \
    public: \
    T(const T&) = delete; \
    T& operator=(const T&) = delete; \
    virtual ~T() = default;

#define LOG_ERROR ESP_LOGE
#define LOG_WARN ESP_LOGW
#define LOG_INFO ESP_LOGI
#define LOG_DEBUG ESP_LOGD
#define LOG_VERBOSE ESP_LOGV

#define DECLARE_APP(APP) \
    void app_bootstrap();                   \
                                            \
    void user_main(void*) {                 \
        APP app;                            \
        app.task_run();                     \
    }                                       \
                                            \
    extern "C" void app_main(void) {        \
        app_bootstrap();                    \
    }
