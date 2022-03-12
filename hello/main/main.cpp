//
// OLED Demo
//

#include "app/application.h"

#include <cmath>

class DemoApp : public Application {

public:
    explicit DemoApp() : Application(), state_{ 0.0f, 0.0f, 0.0f, 0.0f } {}

    void init() {

        LOG_INFO("app", "Init called");

        auto display = get_display();

        state_.x = 0.0f;
        state_.vx = 4.0f;
        state_.y = 0.0f;
        state_.vy = 0.0f;

        LOG_INFO("app", "%0.3f %0.3f %0.3f %0.3f",  state_.x, state_.vx, state_.y, state_.vy);

        display->select_font(1);

        display->stop_scroll();

        display->clear();
        display->draw_string(0, 0, "Hello, world!", oled::WHITE, oled::BLACK);
        display->refresh(true);

        display->start_scroll_horizontal(0, 1, true, 0b111);
        sleep(750);
        display->stop_scroll();

        display->set_page_lock(0);
        display->set_page_lock(1);
    }

    void update(uint32_t frame_counter, uint32_t cycle_counter) {

        auto display = get_display();

        display->clear();

        state_.x += state_.vx;
        if (state_.x < 0.0f) {
            state_.x = 0.0f;
            state_.vx = std::fabs(state_.vx);
        }

        if (state_.x > 127.0f) {
            state_.x = 127.0f;
            state_.vx = -std::fabs(state_.vx);
        }

        state_.y += state_.vy;
        if (state_.y > 63.0f) {
            state_.y = 63.0f;
            state_.vy = -13.0f;
        }

        state_.vy += 1.5f;

        //LOG_INFO("app", "%0.3f %0.3f %0.3f %0.3f",  state_.x, state_.vx, state_.y, state_.vy);

        //display->draw_pixel((int8_t)state_.x, 63-(int8_t)state_.y, oled::WHITE);
        display->draw_pixel((int8_t)state_.x, (int8_t)state_.y, oled::WHITE);

        display->refresh();
    }

    private:
        typedef struct state_t {
            float x;
            float y;
            float vx;
            float vy;
        } state_t;

        state_t state_;

    _NOCOPY(DemoApp)
};

extern "C" void app_main(void) {
    DemoApp app;
    app.run();
}
