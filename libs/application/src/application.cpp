//
// Application
//
#include "application/application.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "graphics/graphics.h"

#include "data.inc"

#define TAG "app"

void user_main(void*);

void app_bootstrap() {
    ESP_LOGI(TAG, "%s", bootmsg);
    ESP_LOGI(TAG, "creating main task");
    xTaskCreatePinnedToCore(user_main, "apploop", 2048, nullptr, 5, nullptr, 1);
}

using namespace application;

Application::Application() {
    timeofs_ = xTaskGetTickCount() * portTICK_PERIOD_MS;
}

uint32_t Application::get_update_counter() const {
    return update_counter_;
}

void Application::exit(int exit_code) {
    exit_code_ = exit_code;
    running_ = false;
}

bool Application::isRunning() const {
    return running_;
}

bool Application::hasError() const {
    return error_;
}

void Application::sleep(uint32_t millis) const {
    vTaskDelay(millis / portTICK_PERIOD_MS);
}

uint32_t Application::get_time() const {
    uint32_t t = xTaskGetTickCount() * portTICK_PERIOD_MS;
    return (t - timeofs_);
}

float Application::get_timef() const {
    return (float)get_time() / 1000.0f;
}

graphics::Display* Application::get_display() {
    return display_;
}

uint32_t Application::get_delta_ms() const {
    return delta_time_ms_;
}

uint32_t Application::get_period_ms() const {
    return period_ms_;
}

void Application::set_period_ms(uint32_t period) {
    period_ms_ = (period / portTICK_PERIOD_MS) * portTICK_PERIOD_MS;
    if (period_ms_ < portTICK_PERIOD_MS) period_ms_ = portTICK_PERIOD_MS;
}

float Application::get_delta_time() const {
    return (float)delta_time_ms_ / 1000.0f;
}

void Application::init() {}

void Application::update() {}

void Application::taskRun() {
    ESP_LOGI(TAG, "main task started");

    ESP_LOGI(TAG, "initialize main task");
    taskInit();
    if (!hasError()) {
        ESP_LOGI(TAG, "enter main task loop");
        taskLoop();
    }

    ESP_LOGE(TAG, "main task aborted. exit code: %d", exit_code_);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_restart();

}

void Application::taskInit() {
    ESP_LOGV(TAG, "application::task_init()");

    if (nullptr != display_) {
        delete display_;
        display_ = nullptr;
    }

    display_ = new graphics::Display();

    if (!display_->init()) {
        ESP_LOGE(TAG, "display initialization failure");
        error_ = true;
        return;
    }

    ESP_LOGI(TAG, "display initialized");

    float display_frequency = display_->get_frequency();
    float display_cycle_time = (display_frequency != 0.0f) ? 1.0f / display_frequency : 0.0f;
    uint32_t display_cycle_time_ms = (uint32_t)(1000.0f * display_cycle_time);
    set_period_ms(display_cycle_time_ms);

    init();
    if (hasError()) {
        ESP_LOGE(TAG, "application initialization failure");
        exit(1);
    }

    ESP_LOGI(TAG, "application initialized");
}

void Application::taskLoop() {
    ESP_LOGV(TAG, "application::task_loop()");

    running_ = true;

    uint32_t cycle_time_ms = get_period_ms();
    uint32_t start_time_ms = get_time();
    update_counter_ = 0;

    uint32_t statistics_time_ms = start_time_ms;
    uint32_t statistics_cycle_time_ms = 5000;
    uint32_t statistics_frame_counter = 0;
    uint32_t statistics_value_counter = 0;

    TickType_t activation_tick = xTaskGetTickCount();
    TickType_t activation_tick_inc = cycle_time_ms / portTICK_PERIOD_MS;
    if (activation_tick_inc < 1) activation_tick_inc = 1;

    ESP_LOGI(TAG, "tick period ms: %d", (int) (portTICK_PERIOD_MS));
    ESP_LOGI(TAG, "task period ms: %d", (int) cycle_time_ms);

    uint32_t last_update_ms = start_time_ms;

    while (true == running_ && false == error_) {
        delta_time_ms_ = (start_time_ms - last_update_ms);
        last_update_ms = start_time_ms;
        start_time_ms = get_time();
        update();
        if (false == running_ || true == error_) break;
        uint32_t now = get_time();
        uint32_t elapsed_time_ms = now - start_time_ms;

        // ESP_LOGI(TAG, "elapsed: %d", elapsed_time_ms);
        update_counter_++;

        statistics_frame_counter++;
        statistics_value_counter += (uint32_t)elapsed_time_ms;
        if (now - statistics_time_ms >= statistics_cycle_time_ms) {
            ESP_LOGI(TAG, "avg. cycle time usage: %d/%d ms, updates/sec: %d",
                     statistics_value_counter / statistics_frame_counter,
                     cycle_time_ms,
                     (statistics_frame_counter * 1000) / (now - statistics_time_ms));

            statistics_time_ms = now;
            statistics_frame_counter = 0;
            statistics_value_counter = 0;
        }

        vTaskDelayUntil(&activation_tick, activation_tick_inc);
    }

    running_ = false;
}
