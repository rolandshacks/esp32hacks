//
// Boing Ball Demo
//

#include <cmath>

#include "application/application.h"
#include "graphics/graphics.h"

#include "./bitmaps.inc"

#include <array>

using namespace graphics;

static const double PI2= 3.14159265358979323846 * 2.0;
static const float PI2f = (float) PI2;
static int __rand_seed = 1;

static std::string scroll_text =
    "                                            " \
    "Hello, world! This is a tribute to the many " \
    "great demos of the home computer area.      " \
    "Greetings to all Amiga and C64 fans!        ";

float rnd()
{
    __rand_seed *= 16807;
    return (float)__rand_seed * 4.6566129e-010f;
}

struct Particle {
    float x {0.0f};
    float y {0.0f};
    float z {0.0f};
};

class RetroDemo : public application::Application {
    public:
        explicit RetroDemo() : Application() {}

        void init() {
            setPeriod(10);
            auto display = getDisplay();
            display->setPartialUpdate(false);
            display->clear();

            initBars();
            initParticles();
        }

        void update() {
            auto display = getDisplay();
            display->clear();

            drawBars();
            drawParticles();
            drawZoomed();
            drawScroller();

            display->update();
        }

        void drawScroller() {
            auto display = getDisplay();
            auto delta = this->getDelta();
            auto start = scroll_text.c_str() + scroll_char_;

            int y = display->height() - display->font()->height;
            int x = - (int) scroll_x_;

            int first_width = 0;
            auto ptr = start;
            while (x < display->width()) {
                char c = *ptr;
                if (c >= 'a' && c <= 'z') c -= ('a' - 'A');
                int char_width = display->drawChar(x, y, c);
                if (0 == first_width) first_width = char_width;
                x += char_width + 1;
                ptr++;
                if (*ptr == '\0') ptr = scroll_text.c_str();
            }

            float delta_pixel = 32.0f * delta;
            if (delta_pixel > 3.0f) {
                delta_pixel = 3.0f;
            }

            scroll_x_ += delta_pixel;
            if (scroll_x_ > (float) (first_width + 1)) {
                scroll_x_ -= (float) (first_width + 1);
                scroll_char_++;
                start++;
                if (*start == '\0') {
                    start = scroll_text.c_str();
                    scroll_char_ = 0;
                }
            }

        }

        void drawZoomed() {
            auto delta = this->getDelta();

            const float speed = 10.0f;
            const float max_z = 40.0f;
            const float min_z = -10.0f;

            if (z1_ > max_z) {
                z1_ = max_z;
                z2_ = max_z + 5.0f;
                vz_ = -1.0f;

            } else if (z1_ < min_z) {
                z1_ = min_z;
                z2_ = min_z;
                vz_ = 1.0f;
            }

            float factor1 = (z1_ > 1.0f) ? 1.0 / z1_ : 1.0f;
            if (factor1 > 0.2f) {
                int x = 64;
                int y = 32 - (int) (12.0f * factor1);
                drawZoomedBitmap(&hello_bitmap, x, y, factor1);
                text_visible_ = true;
            } else {
                text_visible_ = false;
            }

            float factor2 = (z2_ > 1.0f) ? 1.0 / z2_ : 1.0f;
            if (factor2 > 0.2f) {
                int x = 64;
                int y = 32 + (int) (12.0f * factor2);
                drawZoomedBitmap(&world_bitmap, x, y, factor2);
            }

            z1_ += speed * vz_ * delta;
            z2_ += speed * vz_ * delta;
        }

        void drawZoomedBitmap(const Bitmap* bitmap, int x, int y, float factor) {
            int w = (int) ((float) bitmap->width() * factor);
            int h = (int) ((float) bitmap->height() * factor);

            Rectangle src(0, bitmap->width(), 0, bitmap->height());
            Rectangle dest(x-w/2, x-w/2+w, y-h/2, y-h/2+h);

            auto display = getDisplay();
            display->drawStretchBitmap(bitmap, src, dest, true);
        }

        void drawBars() {

            auto display = getDisplay();
            auto delta = this->getDelta();

            bars_y_ += bars_vy_ * delta;
            if (bars_y_ > bars_ymax_) {
                bars_y_ = bars_ymax_;
                bars_vy_ = 0.0f;
            }

            if (text_visible_) {
                if (bars_ymax_ == 0.0f) {
                    bars_ymax_ = 80.0f;
                    bars_vy_ = 50.0f;
                }
            } else {
                if (bars_ymax_ > 0.0f) {
                    bars_ymax_ = 0.0f;
                    bars_y_ = -80.0f;
                    bars_vy_ = 50.0f;
                }
            }

            for (auto& v : bars_) {

                int y = (int) bars_y_ + 31 + (20.0f * std::cos(v));
                int sz = (int) (4.0f * (0.5f + std::sin(v)/2.0f));

                v += delta * 4.5f;
                if (v >= PI2) v -= PI2;

                auto w = display->width()-1;

                int i=y-sz/2;

                if (sz>=3) { display->drawPatternHorizontalLine(0, i, w, 0x55555555); ++i; }
                if (sz>=2) { display->drawPatternHorizontalLine(0, i, w, 0xaaaaaaaa); ++i; }
                display->drawHorizontalLine(0, i, w); ++i;
                if (sz>=1) { display->drawHorizontalLine(0, i, w); ++i; }
                if (sz>=2) { display->drawPatternHorizontalLine(0, i, w, 0xaaaaaaaa); ++i; }
                if (sz>=3) { display->drawPatternHorizontalLine(0, i, w, 0x55555555); ++i; }
            }

        }

        void initBars() {
            for (int i=0; i<bars_.size(); i++) {
                float j = ((float) i) / (float) bars_.size();
                bars_[i] = j * PI2f / 2.0f;
            }
        }

        void drawParticles() {

            auto display = getDisplay();
            auto delta = this->getDelta();

            int w = display->width();
            int h = display->height();

            int center_x = w / 2;
            int center_y = h / 2;


            for (auto& particle : particles_) {

                particle.z -= 3.0f * delta;
                if (particle.z <= 0.0f) {
                    resetParticle(particle);
                }

                if (particle.z <= 0.0f || particle.z >= 3.5f) {
                    continue;
                }

                float factor = 1.0f / particle.z;
                int x = center_x + particle.x * factor;
                int y = center_y + particle.y * factor;

                if (x < 0 || x >= w || y < 0 || y >= h) {
                    continue;
                }

                display->drawPixel(x, y);
            }
        }

        void initParticles() {
            for (auto& particle : particles_) {
                resetParticle(particle);
                particle.z = 2.51f + rnd() * 2.5f;
            }
        }

        void resetParticle(Particle& particle) {
            particle.x =  150.0f * rnd();
            particle.y =  150.0f * rnd();
            particle.z = 5.0f;
        }

    private:
        std::array<Particle, 250> particles_;
        std::array<float, 4> bars_;

        float bars_y_{-80.0f};
        float bars_vy_{50.0f};
        float bars_ymax_{0.0f};

        bool text_visible_{false};
        float z1_{50.0f};
        float z2_{55.0f};
        float vz_{-1.0f};

        float scroll_x_{0.0f};
        int scroll_char_{0};

    _NODEFAULTS(RetroDemo)
};

DECLARE_APP(RetroDemo);
