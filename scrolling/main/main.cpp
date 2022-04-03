//
// Graphics Demo
//

#include <cmath>

#include "application/application.h"
#include "graphics/graphics.h"

const char* LABEL_TOP = "ESP32 HACKS";
const char* LABEL_MID = "Hello, world!";
const char* LABEL_BOTTOM = "Scroll Demo";

class ScrollingDemo : public application::Application {
   public:
    explicit ScrollingDemo() : Application() {}

    void init() {
        auto display = getDisplay();
        display->clear();

        // draw some graphics to be scrolled around...

        int w = display->width();
        int h = display->height();

        auto font = display->setBuiltinFont(0);    // select small font

        scroll_y0_ = font->height + 2;
        scroll_y1_ = h - font->height - 3;

        draw_string_centered(1, LABEL_TOP);
        display->drawHorizontalLine(0, scroll_y0_, w);
        display->drawHorizontalLine(0, scroll_y1_, w);
        draw_string_centered(scroll_y1_+2, LABEL_BOTTOM);

        font = display->setBuiltinFont(3);    // select bigger font
        int y_center = (h - font->height)/2;
        display->drawString(0, y_center, LABEL_MID);

        display->update(true);
    }

    void update() {

        // Important:
        //  - no drawing allowed during activated scrolling
        //  - always stop scrolling before starting a new scroll configuration

        auto display = getDisplay(); // get display reference

        if (0 == animation_counter_) {
            display->startHorizontalScrolling(scroll_start_page_, scroll_end_page, true, scroll_speed_);   // scroll right
        } else if (step_frames_ == animation_counter_) {
            display->startHorizontalScrolling(scroll_start_page_, scroll_end_page, false, scroll_speed_);  // scroll left
        } else if (step_frames_*2 == animation_counter_) {
            display->startDiagonalScrolling(scroll_start_page_, scroll_end_page, scroll_y0_+1, scroll_y1_-1, true, scroll_speed_, 1);    // scroll right and vertical
        } else if (step_frames_*3 == animation_counter_) {
            display->startDiagonalScrolling(scroll_start_page_, scroll_end_page, scroll_y0_+1, scroll_y1_-1, false, scroll_speed_, 1);  // scroll left and vertical
        }

        animation_counter_ = (animation_counter_ + 1) % (step_frames_ * 4);
    }

   private:
    void draw_string_centered(int y, const char* str) {
        // little helper to draw centered text
        auto display = getDisplay();
        int x = (display->width() - display->measureString(str)) / 2;
        display->drawString(x, y, str);
    }

   private:
    uint8_t scroll_speed_{0b000}; // scroll speed: 0b000 = update every 6 frames
    uint32_t animation_counter_{0};
    const int scroll_start_page_{1};
    const int scroll_end_page{6};
    const int step_frames_{400};
    int scroll_y0_{0};
    int scroll_y1_{0};

    _NODEFAULTS(ScrollingDemo)
};

DECLARE_APP(ScrollingDemo);
