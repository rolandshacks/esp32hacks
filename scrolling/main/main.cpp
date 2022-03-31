//
// Graphics Demo
//

#include <cmath>

#include "application/application.h"
#include "graphics/graphics.h"

const char* LABEL_TOP = "ESP32 HACKS";
const char* LABEL_MID = "Hello, world!";
const char* LABEL_BOTTOM = "Scroll Demo";

class HelloGraphics : public application::Application {
   public:
    explicit HelloGraphics() : Application() {}

    void init() {
        auto display = get_display();
        display->stop_scroll();
        display->clear();

        // draw some graphics to be scrolled around...

        int w = display->get_width();
        int h = display->get_height();

        display->select_font(0);    // select small font

        scroll_y0_ = display->get_font_height()+2;
        scroll_y1_ = h-display->get_font_height()-3;

        draw_string_centered(1, LABEL_TOP);
        display->draw_hline(0, scroll_y0_, w, graphics::WHITE);
        display->draw_hline(0, scroll_y1_, w, graphics::WHITE);
        draw_string_centered(scroll_y1_+2, LABEL_BOTTOM);

        display->select_font(3);    // select bigger font
        int y_center = (h - display->get_font_height())/2;
        display->draw_string(0, y_center, LABEL_MID, graphics::WHITE, graphics::BLACK);

        display->refresh(true);
    }

    void update() {

        // Important:
        //  - no drawing allowed during activated scrolling
        //  - always stop scrolling before starting a new scroll configuration

        auto display = get_display(); // get display reference

        if (0 == animation_counter_) {
            display->stop_scroll();
            display->start_scroll_horizontal(scroll_start_page_, scroll_end_page, true, scroll_speed_);   // scroll right
        } else if (step_frames_ == animation_counter_) {
            display->stop_scroll();
            display->start_scroll_horizontal(scroll_start_page_, scroll_end_page, false, scroll_speed_);  // scroll left
        } else if (step_frames_*2 == animation_counter_) {
            display->stop_scroll();
            display->start_scroll(scroll_start_page_, scroll_end_page, scroll_y0_+1, scroll_y1_-1, true, scroll_speed_, 1);    // scroll right and vertical
        } else if (step_frames_*3 == animation_counter_) {
            display->stop_scroll();
            display->start_scroll(scroll_start_page_, scroll_end_page, scroll_y0_+1, scroll_y1_-1, false, scroll_speed_, 1);  // scroll left and vertical
        }

        animation_counter_ = (animation_counter_ + 1) % (step_frames_ * 4);
    }

   private:
    void draw_string_centered(int y, const char* str) {
        // little helper to draw centered text
        auto display = get_display();
        int x = (display->get_width() - display->measure_string(str)) / 2;
        display->draw_string(x, y, str, graphics::WHITE, graphics::BLACK);
    }

   private:
    uint8_t scroll_speed_{0b000}; // scroll speed: 0b000 = update every 6 frames
    uint32_t animation_counter_{0};
    const int scroll_start_page_{1};
    const int scroll_end_page{6};
    const int step_frames_{400};
    int scroll_y0_{0};
    int scroll_y1_{0};

    _NODEFAULTS(HelloGraphics)
};

DECLARE_APP(HelloGraphics);
