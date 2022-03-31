//
// Sim
//

#include "freertos/FreeRTOS.h"

#include <chrono>
#include <iostream>
#include <thread>

#include "freertos/task.h"

bool app_update();

static void __milli_sleep(uint32_t millis) {
    uint32_t sleep_time = millis;
    uint32_t sleep_inc = 5;
    while (sleep_time >= 0) {
        app_update();
        uint32_t delta = (sleep_time > sleep_inc) ? sleep_inc : sleep_time;
        std::this_thread::sleep_for(std::chrono::milliseconds(delta));
        sleep_time -= delta;
        if (sleep_time < 1) break;
    }
}

TickType_t ticks_offset = 0;

volatile TickType_t xTaskGetTickCount(void) {
    TickType_t ticks =
        std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(portTICK_PERIOD_MS);

    if (0 == ticks_offset) {
        ticks_offset = ticks;
    }

    return (ticks - ticks_offset);
}

BaseType_t xTaskCreatePinnedToCore(TaskFunction_t pvTaskCode, const char* const pcName, const uint32_t usStackDepth,
                                   void* const pvParameters, UBaseType_t uxPriority, TaskHandle_t* const pvCreatedTask,
                                   const BaseType_t xCoreID) {
    pvTaskCode(pvParameters);
    return 0;
}

void vTaskDelay(const TickType_t xTicksToDelay) {
    uint32_t millis = xTicksToDelay * portTICK_PERIOD_MS;
    __milli_sleep(millis);
}

void vTaskDelayUntil(TickType_t *pxPreviousWakeTime, const TickType_t xTimeIncrement) {
    TickType_t next_wakeup = *pxPreviousWakeTime + xTimeIncrement;

    auto now = xTaskGetTickCount();
    if (next_wakeup > now) {
        uint32_t millis = (next_wakeup - now) * portTICK_PERIOD_MS;
        __milli_sleep(millis);
    } else {
        next_wakeup = now;
    }

    *pxPreviousWakeTime = next_wakeup;
}
