//
// Boing Ball Demo
//

#include <cmath>

#include "application/application.h"
#include "graphics/graphics.h"

#include "./bitmaps.inc"

const graphics::Bitmap* bitmaps[] = {
    &ball0_bitmap,
    &ball1_bitmap,
    &ball2_bitmap,
    &ball3_bitmap,
    &ball4_bitmap,
    &ball5_bitmap
};

const int num_bitmaps = sizeof(bitmaps) / sizeof(bitmaps[0]);

class BoingBall : public application::Application {
   public:
    explicit BoingBall()
        : Application() {}

    void init() {
        LOG_INFO("app", "Boing ball demo - inspired by Amiga CES Demo 1984!");

        setPeriod(10);

        auto display = getDisplay();                       // get display reference

        auto bitmap = bitmaps[0];                           // get data from first bitmap

        state_.x = 0.0f;                                    // initialize ball state
        state_.vx = 25.0f;
        state_.y = 0.0f;
        state_.vy = 0.0f;
        state_.vy_acc = 100.0f;
        state_.vy_init = -70.0f;
        state_.a = 0.0f;
        state_.va = 28.0f;
        state_.max_x = (float) (display->width() - bitmap->width());
        state_.max_y = (float) (display->height() - bitmap->height());

        display->clear();                                   // clear display
    }

    void update() {
        auto display = getDisplay();                       // get display reference

        auto w = display->width();
        auto h = display->height();

        display->clear();                                   // clear display

        // draw background grid
        display->drawHorizontalLine(8, 58, w-8);
        display->drawHorizontalLine(6, 61, w-6);
        for (int y = h-9; y >= 0; y -= 11) {
            display->drawHorizontalLine(10, y, w-10);
        }
        for (int i = 0; i < 6; i++) {
            display->drawVerticalLine(w / 2 - i * 11, 0, h-9);
            display->drawLine(w / 2 - i * 11, h-9, w / 2 - i * 12, h-1);
            display->drawVerticalLine(w / 2 + i * 11, 0, h-9);
            display->drawLine(w / 2 + i * 11, h-9, w / 2 + i * 12, h-1);
        }

        if (0 != getUpdateCounter()) {
            // Don't completely redraw after first update
            // updating the display is the most time-consuming part!!
            display->device()->clearRegions();
        }

        auto bitmap = bitmaps[(int)state_.a];               // get bitmap data

        // mark old ball position to fix and refresh the background
        display->device()->markRegion((uint8_t)state_.x,
                                      (uint8_t)state_.x + bitmap->width(),
                                      (uint8_t)state_.y,
                                      (uint8_t)state_.y + bitmap->height());

        updateState(getDelta());                            // update ball state

        display->drawBitmap(bitmap, (uint8_t)state_.x, (uint8_t)state_.y);

        display->update();                                 // refresh display
    }

   private:
    void updateState(float delta_time) {
        state_.x += state_.vx * delta_time;                 // update x-pos
        if (state_.x < 0.0f) {
            state_.x = 0.0f;
            state_.vx = std::fabs(state_.vx);
        }

        if (state_.x > state_.max_x) {
            state_.x = state_.max_x;
            state_.vx = -std::fabs(state_.vx);
        }

        state_.y += state_.vy * delta_time;                 // update y-pos
        if (state_.y > state_.max_y) {                      // boing!
            state_.y = state_.max_y;
            state_.vy = state_.vy_init;
        }

        if (state_.y < 0.0f) {
            state_.y = 0.0f;
            state_.vy = 0.0f;
        }

        state_.vy += state_.vy_acc * delta_time;            // let y fall down

        float max_anim = (float) num_bitmaps;
        state_.a += delta_time * ((state_.vx < 0.0f) ? state_.va : -state_.va);  // update rotation
        if (state_.a >= max_anim) {
            state_.a = fmod(state_.a, max_anim);
        } else if (state_.a < 0.0f) {
            state_.a = max_anim - fmod(-state_.a, max_anim);
        }
    }

   private:
    typedef struct state_t {                                // state for ball
        float x;
        float y;
        float vx;
        float vy;
        float vy_acc;
        float vy_init;
        float a;
        float va;
        float max_x;
        float max_y;
    } state_t;

    state_t state_;

    _NODEFAULTS(BoingBall)
};

DECLARE_APP(BoingBall);
