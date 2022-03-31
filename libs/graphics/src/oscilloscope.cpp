//
// Oscilloscope
//
#include "graphics/oscilloscope.h"
#include "graphics/graphics.h"

#include <esp_log.h>

#include <memory.h>
#include <string>

using namespace graphics;

void Oscilloscope::init() {
    ESP_LOGI("oscilloscope", "Oscilloscope initialized");
    buffer_size_ = 128;
    clear();
}

void Oscilloscope::clear() {
    buffer_.resize(buffer_size_, 0);
    buffer_ofs_ = buffer_read_ofs_ = buffer_usage_ = 0;
    min_value_ = max_value_ = value_ = 0;
    first_value_ = true;
}

void Oscilloscope::add(int value) {
    value_ = value;

    if (first_value_ || value < min_value_) min_value_ = value;
    if (first_value_ || value > max_value_) max_value_ = value;
    first_value_ = false;

    buffer_[buffer_ofs_] = value;
    buffer_read_ofs_ = buffer_ofs_;
    buffer_ofs_ = (buffer_ofs_ + 1) % buffer_size_;

    if (buffer_usage_ < buffer_size_) buffer_usage_++;
}

int Oscilloscope::getValue() const { return value_; }

void Oscilloscope::draw(graphics::Display* display) {
    draw(display, 0, 0, display->get_width(), display->get_height(), true, 0,
         0);
}

void Oscilloscope::draw(graphics::Display* display, int pos_x, int pos_y, int w,
                        int h, bool show_text, int text_pos_x, int text_pos_y) {
    if (show_text) {
        sprintf(textbuffer_, "DATA: %d", value_);
        display->draw_string(text_pos_x, text_pos_y, textbuffer_,
                             graphics::WHITE, graphics::BLACK);
    }

    int height = (h > 0) ? h : display->get_height();
    int width = (w > 0) ? w : display->get_width();
    if (width > buffer_size_) width = buffer_size_;

    int range = max_value_ - min_value_;

    size_t ofs = buffer_read_ofs_;
    size_t usage = buffer_usage_;

    int x = width;
    while (x > 0 && usage > 0) {
        x--;
        usage--;

        int v = buffer_[ofs];
        if (ofs > 0) {
            ofs--;
        } else {
            ofs = buffer_size_ - 1;
        }

        int y =
            (range > 0) ? height - 1 - ((v - min_value_) * height / range) : 0;
        display->draw_pixel(pos_x + x, pos_y + y, graphics::WHITE);
    }
}
