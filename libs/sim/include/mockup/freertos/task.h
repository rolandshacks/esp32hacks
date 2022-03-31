#pragma once

volatile TickType_t xTaskGetTickCount(void);

BaseType_t xTaskCreatePinnedToCore(TaskFunction_t pvTaskCode,
                                   const char* const pcName,
                                   const uint32_t usStackDepth,
                                   void* const pvParameters,
                                   UBaseType_t uxPriority,
                                   TaskHandle_t* const pvCreatedTask,
                                   const BaseType_t xCoreID);

void vTaskDelay(const TickType_t xTicksToDelay);
void vTaskDelayUntil(TickType_t *pxPreviousWakeTime, const TickType_t xTimeIncrement);
