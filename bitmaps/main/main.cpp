//
// Boing Ball Demo
//

#include <cmath>

#include "application/application.h"
#include "graphics/graphics.h"

#include "./bitmaps.inc"

const double PI2 = 3.14159265358979323846 * 2.0;

class BitmapDemo : public application::Application {
   public:
    explicit BitmapDemo() : Application() {}

    void init() {
        set_period_ms(50);

        auto display = get_display();
        display->enable_partial_updates(false);
        display->clear();
    }

    void update() {
        auto display = get_display();

        const auto& background = background_bitmap_info;
        display->draw_bitmap(&background, 0, 0, false);

        w = fmod(w + get_delta_time() * 4.0, PI2);

        auto x = 20.0 * cos(w);
        auto y = 20.0 * sin(w);

        const auto& bitmap = hello_bitmap_info;
        display->draw_bitmap(
            &bitmap,
            (display->get_width() - bitmap.width) / 2 + (int) x,
            (display->get_height() - bitmap.height) / 2 + (int) y);

        display->refresh();
    }

   private:
    double w{0.0};

    _NODEFAULTS(BitmapDemo)
};

DECLARE_APP(BitmapDemo);
