//
// Oscilloscope
//
#pragma once

#include <vector>

#include "driver/adc.h"

namespace graphics {

class Display;

class Oscilloscope {
   public:
    void init();
    void add(int value);
    void clear();
    int getValue() const;

   public:
    void draw(graphics::Display* display);
    void draw(graphics::Display* display, int pos_x, int pos_y, int w, int h,
              bool show_text, int text_pos_x, int text_pos_y);

   private:
    int fetch_data();

   private:
    char textbuffer_[128];
    int value_{0};
    int min_value_{0};
    int max_value_{0};
    bool first_value_{false};

    std::vector<int> buffer_;
    size_t buffer_size_{0};
    size_t buffer_ofs_{0};
    size_t buffer_usage_{0};
    size_t buffer_read_ofs_{0};
};

}  // namespace graphics
