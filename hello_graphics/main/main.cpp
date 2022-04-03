//
// Graphics Demo
//

#include <cmath>

#include "application/application.h"
#include "graphics/graphics.h"

#include <vector>

typedef struct coords_ {
    float x;
    float y;

    coords_(float _x, float _y) : x(_x), y(_y) {}
} coords_t;

class Entity {

    public:
        Entity(float x, float y, float vx, float vy) :
            pos{x, y}, speed{vx, vy} {}

        void update(float delta_time) {
            pos.x += speed.x * delta_time;  // update x-pos
            if (pos.x < 0.0f) {
                pos.x = 0.0f;
                speed.x = std::fabs(speed.x);
            }

            if (pos.x > 127.0f) {
                pos.x = 127.0f;
                speed.x = -std::fabs(speed.x);
            }

            pos.y += speed.y * delta_time;  // update y-pos
            if (pos.y > 63.0f) {
                pos.y = 63.0f;
                speed.y = -200.0f;
            }

            speed.y += 450.0f * delta_time;  // let y fall down
        }

    public:
        void dump() {
            LOG_INFO("dump", "%0.3f %0.3f %0.3f %0.3f", pos.x, pos.y, speed.x, speed.y);
        }

    public:
        coords_t pos;
        coords_t speed;

};

class HelloGraphics : public application::Application {
   public:
    explicit HelloGraphics() : Application(),
        pos1_{0.0f, 32.0f, 137.0f, 0.0f},
        pos2_{10.0f, 12.0f, 171.0f, 0.0f}
    {
        for (auto i=0; i<num_last_pos_; i++) {
            last_pos_.emplace_back(-1, -1);
            last_pos_.emplace_back(-1, -1);
        }
    }

    void init() override {

        setPeriod(40);                                         // set 40 ms cycle / 25 fps
        showStatistics(true);                                  // enable statistics overlay

        auto display = getDisplay();                           // get display reference

        display->setBuiltinFont(1);                            // set font
        display->clear();                                      // clear display
        display->drawString(0, 0, "Hello, world!");            // display text
        display->update(true);                                 // update display

        display->startHorizontalScrolling(0, 1, true, 0b111);  // turn on scrolling
        sleep(750);                                            // drawing forbidden while scrolling!!
        display->stopScrolling();                              // turn off scrolling

        display->lockPage(0);                                  // write-protect display area
        display->lockPage(1);                                  // write-protect display area
    }

    void update() override {
        auto display = getDisplay();  // get display reference

        display->clear();  // clear display (ignore locked areas)

        // update state
        float delta_time = getDelta();
        pos1_.update(delta_time);
        pos2_.update(delta_time);

        // update trace
        last_pos_[last_pos_ofs_] = pos1_.pos;
        last_pos_[last_pos_ofs_+1] = pos2_.pos;
        last_pos_ofs_ = (last_pos_ofs_ + 2) % (num_last_pos_ * 2);

        // draw lines
        for (auto i=0; i<num_last_pos_; i++) {
            size_t ofs = ((last_pos_ofs_ + i) % num_last_pos_) * 2;
            display->drawLine((int8_t)last_pos_[ofs].x, (int8_t)last_pos_[ofs].y,
                              (int8_t)last_pos_[ofs+1].x, (int8_t)last_pos_[ofs+1].y);
        }

        display->update();  // refresh display
    }

   private:

    Entity pos1_;
    Entity pos2_;

    std::vector<coords_t> last_pos_;
    const size_t num_last_pos_{16};
    size_t last_pos_ofs_{0};

    _NODEFAULTS(HelloGraphics)
};

DECLARE_APP(HelloGraphics);
