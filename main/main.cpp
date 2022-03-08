//
// OLED Demo
//

#include <adk>
#include <cmath>

class DemoApp : public Application {

public:
    explicit DemoApp() : Application(), state_{ 0.0f, 0.0f, 0.0f, 0.0f } {}

    void init() {

        auto display = get_display();

        state_.x = 0.0f;
        state_.vx = 1.0f;
        state_.y = 0.0f;
        state_.vy = 0.0f;

        LOG_INFO("app", "Init called");

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

    void scrolling_demo() {

        auto display = get_display();

        static int vertical_offset = 0;

        int w = display->get_width();
        int h = display->get_height();

        state_.x += state_.vx;
        if (state_.x < 0.0f) {
            state_.x = 0.0f;
            state_.vx = std::fabs(state_.vx);
        }

        if (state_.x > 127.0) {
            state_.x = 127.0;
            state_.vx = -std::fabs(state_.vx);
        }

        state_.y += state_.vy;
        if (state_.y > 63.0) {
            state_.y = 63.0;
            state_.vy = -12.0f;
        }

        state_.vy += 1.5f;

        vertical_offset++;
        if (vertical_offset >= h) vertical_offset = 0;

        display->set_vertical_offset(vertical_offset);
        display->draw_hline(0, vertical_offset, w, oled::BLACK);
        display->draw_hline(0, (vertical_offset + h - 1) % h, w, oled::BLACK);
        display->draw_hline(state_.x - state_.y/2, (vertical_offset + h - 2) % h, state_.y, oled::WHITE);

        display->refresh();

    }

    void update(uint32_t frame_counter, uint32_t cycle_counter) {

        auto display = get_display();

        display->clear();

        //display->draw_string(60, 0, "Hello", oled::WHITE, oled::BLACK);
        //display->fill_rectangle(0, 16, display->get_width(), display->get_height(), oled::BLACK);

        state_.x += state_.vx;
        if (state_.x < 0.0f) {
            state_.x = 0.0f;
            state_.vx = std::fabs(state_.vx);
        }

        if (state_.x > 127.0) {
            state_.x = 127.0;
            state_.vx = -std::fabs(state_.vx);
        }

        state_.y += state_.vy;
        if (state_.y > 63.0) {
            state_.y = 63.0;
            state_.vy = -12.0f;
        }

        state_.vy += 1.5f;

        display->draw_pixel((int8_t)state_.x, 63-(int8_t)state_.y, oled::WHITE);
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
