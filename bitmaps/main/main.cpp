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
        setPeriod(50);

        auto display = getDisplay();
        display->setPartialUpdate(false);
        display->clear();
    }

    void update() {
        auto display = getDisplay();

        const auto& background = background_bitmap_info;

        //for (int i=0; i<200; i++)
        display->drawBitmap(&background, 0, 0, false);

        w = fmod(w + getDelta() * 4.0, PI2);

        auto x = 20.0 * cos(w);
        auto y = 20.0 * sin(w);

        const auto& bitmap = hello_bitmap_info;
        display->drawBitmap(
            &bitmap,
            (display->width() - bitmap.width) / 2 + (int) x,
            (display->height() - bitmap.height) / 2 + (int) y);

        display->update();
    }

   private:
    double w{0.0};

    _NODEFAULTS(BitmapDemo)
};

DECLARE_APP(BitmapDemo);
