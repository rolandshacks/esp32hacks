//
// Application
//
#include "app/bits.h"
#include "app/application.h"

#include <math.h>

Application::Application() :
    running_(false),
    error_(false),
    timeofs_(0),
    display_(nullptr) {
    timeofs_ = xTaskGetTickCount() * portTICK_PERIOD_MS;
}

void Application::stop() {
    running_ = false;
}

bool Application::is_running() const {
    return running_;
}

bool Application::has_error() const {
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
    return (float) get_time() / 1000.0f;
}

Display* Application::get_display() {
    return display_;
}

void Application::task_init() {
    display_ = new Display();

    if (!display_->init()) {
        ESP_LOGE("app", "Task display initialization failure");
        error_ = true;
        return;
    }

    init();
    if (has_error()) {
        ESP_LOGE("app", "Task initialization failure");
        return;
    }

    ESP_LOGI("app", "Task initialized");
}

void Application::task_loop() {

    running_ = true;

    float display_cycle_time = display_->get_frame_cycle_time();
    float display_cycles_per_sec = 1.0f / display_cycle_time;

    uint32_t cycle_time_ms = (uint32_t) (1000.0f * display_cycle_time * 25.0f);

    // ESP_LOGI("app", "cycle time: %d (HW cycles/sec: %0.2f)", cycle_time_ms, display_cycles_per_sec);

    uint32_t start_time_ms = get_time();
    uint32_t frame_counter = 0;

    uint32_t statistics_time_ms = start_time_ms;
    uint32_t statistics_cycle_time_ms = 5000;
    uint32_t statistics_frame_counter = 0;
    uint32_t statistics_value_counter = 0;

    int last_time_ms = 0;
    int delta_time_ms = 0;

    float display_frame_counter = 0.0f;

    while (true == running_ and false == error_) {

        int current_time_ms = get_time();
        if (0 != last_time_ms) {
            delta_time_ms = current_time_ms - last_time_ms;
        } else {
            delta_time_ms = 0;
        }

        last_time_ms = current_time_ms;

        display_frame_counter += (float) delta_time_ms * display_cycles_per_sec / 1000.0f;
        uint32_t display_frame_counter_int = (uint32_t) display_frame_counter;

        update(frame_counter, display_frame_counter_int);

        int elapsed_time_ms = get_time() - current_time_ms;

        //ESP_LOGI("run", "elapsed: %d", elapsed_time_ms);

        statistics_frame_counter++;
        statistics_value_counter += (uint32_t) elapsed_time_ms;
        if (current_time_ms - statistics_time_ms >= statistics_cycle_time_ms) {

            ESP_LOGI("run", "Avg. cycle time usage: %d/%d ms, frame #%d, cycle #%d",
                statistics_value_counter/statistics_frame_counter,
                cycle_time_ms,
                frame_counter,
                display_frame_counter_int
            );

            statistics_time_ms = current_time_ms;
            statistics_frame_counter = 0;
            statistics_value_counter = 0;
        }

        int sleep_time = (cycle_time_ms > elapsed_time_ms) ? cycle_time_ms - elapsed_time_ms : 1;
        sleep(sleep_time);

        //ESP_LOGW("run", "Sleep time: %d", sleep_time);

        frame_counter++;
    }

    running_ = false;

}

void Application::init() {
    ;
}

void Application::update(uint32_t frame_counter, uint32_t cycle_counter) {
    ;
}

void Application::task_entry(void* pvParameters) {

    ESP_LOGI("app", "Task started");

    Application* ptrApp = (Application*) pvParameters;

    ptrApp->task_init();
    if (!ptrApp->has_error()) {
        ptrApp->task_loop();
    }

    for (;;) {
        ESP_LOGE("app", "Task aborted unexpectedly");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void Application::run() {

    ESP_LOGI("app", "Started");

    ESP_LOGI("app", "Creating task");
    xTaskCreatePinnedToCore(Application::task_entry, "apploop", 2048, (void*) this, 5, nullptr, 1);
}
