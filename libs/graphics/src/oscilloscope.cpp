//
// Oscilloscope
//
#include "graphics/oscilloscope.h"
#include "graphics/device.h"
#include "graphics/graphics.h"

#include <esp_log.h>
#include <cmath>
#include <algorithm>
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
    draw(display, 0, 0, display->width()-1, display->height()-1, true, 0, 0);
}

void Oscilloscope::draw(graphics::Display* display,
                        int x1, int y1, int x2, int y2,
                        bool show_text, int text_pos_x, int text_pos_y) {

    if (show_text) {
        sprintf(textbuffer_, "DATA: %d", value_);
        display->drawString(text_pos_x, text_pos_y, textbuffer_);
    }

    int w = 1 + ((x2 >= x1) ? x2 - x1 : x1 - x2);
    int h = 1 + ((y2 >= y1) ? y2 - y1 : y1 - y2);

    int height = (h > 0) ? h : display->height();
    int width = (w > 0) ? w : display->width();
    if (width > buffer_size_) width = buffer_size_;

    int range = max_value_ - min_value_;
    int center = 0;
    if (center < min_value_) center = min_value_;
    if (center > max_value_) center = max_value_;

    size_t ofs = buffer_read_ofs_;
    size_t usage = buffer_usage_;

    int last_x = 0;
    int last_y = 0;
    int count = 0;

    int i = width;
    while (i > 0 && usage > 0) {
        i--;
        usage--;

        int v = buffer_[ofs];
        if (ofs > 0) {
            ofs--;
        } else {
            ofs = buffer_size_ - 1;
        }

        int y_center = y1 + ((range > 0) ? height - 1 - ((center - min_value_) * height / range) : 0);
        display->drawHorizontalLine(x1, y_center, x2);

        int x = x1 + i;
        int y = y1 + ((range > 0) ? height - 1 - ((v - min_value_) * height / range) : 0);

        if (count > 0) {
            display->drawLine(last_x, last_y, x, y);
        }

        last_x = x;
        last_y = y;

        count++;
    }
}
