//
// Application
//
#include "application/application.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "graphics/graphics.h"

#include "data.inc"

#define TAG "app"

using namespace application;

Application::Application() {
    timeofs_ = xTaskGetTickCount() * portTICK_PERIOD_MS;
}

void Application::bootstrap(void (*task_entry)(void*)) {
    ESP_LOGI(TAG, "%s", bootmsg);
    ESP_LOGI(TAG, "creating main task");
    xTaskCreatePinnedToCore(task_entry, "apploop", 1024*16, nullptr, 5, nullptr, 1);
}

uint32_t Application::getUpdateCounter() const {
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

uint32_t Application::getMillis() const {
    uint32_t t = xTaskGetTickCount() * portTICK_PERIOD_MS;
    return (t - timeofs_);
}

float Application::getTime() const {
    return (float)getMillis() / 1000.0f;
}

graphics::Display* Application::getDisplay() {
    return display_;
}

uint32_t Application::getDeltaMillis() const {
    return delta_time_ms_;
}

float Application::getDelta() const {
    return (float)delta_time_ms_ / 1000.0f;
}

uint32_t Application::getPeriod() const {
    return period_ms_;
}

void Application::setPeriod(uint32_t period_ms) {
    period_ms_ = ((period_ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS) * portTICK_PERIOD_MS;
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

    float display_frequency = display_->device()->frequency();
    float display_cycle_time = (display_frequency != 0.0f) ? 1.0f / display_frequency : 0.0f;
    uint32_t display_cycle_time_ms = (uint32_t)(1000.0f * display_cycle_time);
    setPeriod(display_cycle_time_ms);

    init();
    if (hasError()) {
        ESP_LOGE(TAG, "application initialization failure");
        exit(1);
    }

    display_->setDeferredUpdate(true);

    ESP_LOGI(TAG, "application initialized");
}

uint32_t Application::getAvgCycleTime() const {
    return avg_cycle_time_ms_;
}

uint32_t Application::getAvgUpdatesPerSecond() const {
    return avg_updates_per_sec_;
}

void Application::taskLoop() {
    ESP_LOGV(TAG, "application::task_loop()");

    running_ = true;

    uint32_t cycle_time_ms = getPeriod();
    uint32_t start_time_ms = getMillis();
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
        start_time_ms = getMillis();
        update();
        renderOverlay();
        if (display_->getDeferredUpdate()) {
            display_->refresh();
        }

        if (false == running_ || true == error_) break;
        uint32_t now = getMillis();
        uint32_t elapsed_time_ms = now - start_time_ms;

        // ESP_LOGI(TAG, "elapsed: %d", elapsed_time_ms);
        update_counter_++;

        statistics_frame_counter++;
        statistics_value_counter += (uint32_t)elapsed_time_ms;
        if (now - statistics_time_ms >= statistics_cycle_time_ms) {
            avg_cycle_time_ms_ = statistics_value_counter / statistics_frame_counter;
            avg_updates_per_sec_ = (statistics_frame_counter * 1000) / (now - statistics_time_ms);

            ESP_LOGI(TAG, "avg. cycle time usage: %d/%d ms, updates/sec: %d",
                     avg_cycle_time_ms_,
                     cycle_time_ms,
                     avg_updates_per_sec_);

            statistics_time_ms = now;
            statistics_frame_counter = 0;
            statistics_value_counter = 0;
        }

        vTaskDelayUntil(&activation_tick, activation_tick_inc);
    }

    running_ = false;
}

void Application::showStatistics(bool show) {
    show_stats_ = show;
}

void Application::renderOverlay() {

    static char buf[32];

    if (!show_stats_) return;

    auto display = getDisplay();

    auto old_font = display->font();

    display->setBuiltinFont(0);

    sprintf(buf, "%d/%d/%d", avg_cycle_time_ms_, getPeriod(), avg_updates_per_sec_);
    display->drawString(0, display->height() - display->font()->height, buf);

    if (!display_->getDeferredUpdate()) {
        display_->refresh();
    }

   display->setFont(old_font);
}
