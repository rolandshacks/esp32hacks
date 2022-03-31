#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define portTICK_PERIOD_MS 10
#define portMAX_DELAY (TickType_t)0xffffffffUL

typedef void* SemaphoreHandle_t;
typedef void* QueueHandle_t;
typedef void* TaskHandle_t;
typedef uint32_t TickType_t;

typedef void (*TaskFunction_t)(void*);
typedef unsigned int UBaseType_t;
typedef int BaseType_t;

#define pdFALSE ((BaseType_t)0)
#define pdTRUE ((BaseType_t)1)

#define pdPASS (pdTRUE)
#define pdFAIL (pdFALSE)

#define pdMS_TO_TICKS(tick) (tick)
