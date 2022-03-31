//
// Boing Ball Demo
//

#include <cmath>

#include "./bitmaps.cpp"
#include "application/application.h"
#include "graphics/graphics.h"

const unsigned char* bitmaps[] = {bitmap_0, bitmap_1, bitmap_2,
                                  bitmap_3, bitmap_4, bitmap_5};

const unsigned char* mask_bitmap = bitmap_6;

class BoingBall : public application::Application {
   public:
    explicit BoingBall()
        : Application(), state_{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f} {}

    void init() {
        LOG_INFO("app", "Boing ball demo");

        auto display = get_display();  // get display reference

        state_.x = 0.0f;  // set start position
        state_.vx = 25.0f;
        state_.y = 0.0f;
        state_.vy = 0.0f;
        state_.a = 0.0f;
        state_.va = 23.0f;

        display->select_font(1);  // set font
        display->stop_scroll();   // stop/init scrolling

        display->clear();  // clear display
    }

    void update() {
        auto display = get_display();  // get display reference

        float delta_time = get_delta_time();

        display->clear();  // clear display (ignore locked areas)

        display->draw_hline(8, 55, 112, graphics::WHITE);  // draw background grid
        display->draw_hline(4, 58, 120, graphics::WHITE);
        display->draw_hline(0, 61, 128, graphics::WHITE);
        for (int y = 0; y < 50; y += 12) {
            display->draw_hline(10, y, 108, graphics::WHITE);
        }
        for (int x = 10; x < 120; x += 12) {
            display->draw_vline(x, 0, 55, graphics::WHITE);
        }

        state_.x += state_.vx * delta_time;  // update x-pos
        if (state_.x < 0.0f) {
            state_.x = 0.0f;
            state_.vx = std::fabs(state_.vx);
        }

        if (state_.x > 127.0f - 32.0f) {
            state_.x = 127.0f - 32.0f;
            state_.vx = -std::fabs(state_.vx);
        }

        state_.y += state_.vy * delta_time;   // update y-pos
        if (state_.y > 31.0f) {               // boing!
            state_.y = 31.0f;
            state_.vy = -55.0f;
        }

        if (state_.y < 0.0f) {
            state_.y = 0.0f;
            state_.vy = 0.0f;
        }

        state_.vy += 55.0f * delta_time;  // let y fall down

        state_.a += delta_time * ((state_.vx < 0.0f) ? state_.va : -state_.va);  // update rotation
        if (state_.a >= 6.0f) {
            state_.a = fmod(state_.a, 6.0f);
        } else if (state_.a < 0.0f) {
            state_.a = 6.0 - fmod(-state_.a, 6.0f);
        }

        const unsigned char* bitmap = bitmaps[(int)state_.a];  // draw ball
        display->draw_masked_bitmap(bitmap, mask_bitmap, (uint8_t)state_.x,
                                    (uint8_t)state_.y, 32, 32, graphics::WHITE,
                                    graphics::BLACK);

        display->refresh();  // refresh display
    }

   private:
    typedef struct state_t {  // state for some moving parts
        float x;
        float y;
        float vx;
        float vy;
        float a;
        float va;
    } state_t;

    state_t state_;

    _NODEFAULTS(BoingBall)
};

DECLARE_APP(BoingBall);
