//
// OLED Demo
//

#include "app/application.h"

#include <cmath>

class HelloGraphics : public Application {

public:
    explicit HelloGraphics() : Application(), state_{ 0.0f, 0.0f, 0.0f, 0.0f } {}

    void init() {

        LOG_INFO("app", "init called");

        auto display = get_display();                           // get display reference

        state_.x = 0.0f;                                        // set start position
        state_.vx = 3.0f;
        state_.y = 0.0f;
        state_.vy = 0.0f;

        display->select_font(1);                                // set font
        display->stop_scroll();                                 // stop/init scrolling

        display->clear();                                       // clear display
        display->draw_string(0, 0, "Hello, world!", oled::WHITE, oled::BLACK);  // display text
        display->refresh(true);                                 // update display

        display->start_scroll_horizontal(0, 1, true, 0b111);    // turn on scrolling
        sleep(750);                                             // blocking wait for scrolling
        display->stop_scroll();                                 // turn off scrolling

        display->set_page_lock(0);                              // write-protect display area
        display->set_page_lock(1);                              // write-protect display area
    }

    void update(uint32_t frame_counter, uint32_t cycle_counter) {

        auto display = get_display();                           // get display reference

        display->clear();                                       // clear display (ignore locked areas)

        state_.x += state_.vx;                                  // update x-pos
        if (state_.x < 0.0f) {
            state_.x = 0.0f;
            state_.vx = std::fabs(state_.vx);
        }

        if (state_.x > 127.0f) {
            state_.x = 127.0f;
            state_.vx = -std::fabs(state_.vx);
        }

        state_.y += state_.vy;                                  // update y-pos
        if (state_.y > 63.0f) {
            state_.y = 63.0f;
            state_.vy = -12.0f;
        }

        state_.vy += 1.5f;                                      // let y fall down

        //LOG_INFO("app", "%0.3f %0.3f %0.3f %0.3f",  state_.x, state_.vx, state_.y, state_.vy);

        display->draw_pixel((int8_t)state_.x, (int8_t)state_.y, oled::WHITE);   // draw pixel to display
        display->refresh();                                     // refresh display
    }

    private:
        typedef struct state_t {                                // state for some moving parts
            float x;
            float y;
            float vx;
            float vy;
        } state_t;

        state_t state_;

    _NOCOPY(HelloGraphics)
};

DECLARE_APP(HelloGraphics);
