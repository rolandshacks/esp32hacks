//
// SSD1306 OLED Display Controller
//
#include "graphics/display.h"

#include <memory.h>
#include <stdint.h>

#include "esp_log.h"

#define MAX_I2C_LIST_LEN 32

#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26               ///< Init rt scroll
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27                ///< Init left scroll
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29  ///< Init diag scroll
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A   ///< Init diag scroll
#define SSD1306_DEACTIVATE_SCROLL 0x2E                     ///< Stop scroll
#define SSD1306_ACTIVATE_SCROLL 0x2F                       ///< Start scroll
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3              ///< Set scroll range

#define SSD1306_DEFAULT_ADDRESS 0x78
#define SSD1306_DEFAULT_PIN_SDA_DATA GPIO_NUM_21
#define SSD1306_DEFAULT_PIN_SCL_CLOCK GPIO_NUM_22
#define SSD1306_DEFAULT_TYPE graphics::SSD1306_128x64

/// Oscillator frequency table for 0xD5 command
static const uint16_t OSC_FREQUENCY_TABLE[] = {270, 279, 289, 298, 314, 326, 337, 352,
                                               372, 391, 409, 431, 451, 477, 506, 536};

using namespace graphics;

Display::Display(gpio_num_t scl, gpio_num_t sda, panel_type_t type, uint8_t address)
    : i2c(nullptr),
      type_(type),
      address_(address),
      buffer_(nullptr),
      buffer_size_(0),
      width_(0),
      height_(0),
      num_pages_(0),
      font_(nullptr),
      partial_updates_enabled_(true),
      frequency_(0.0f) {
    i2c = new sys::I2C(scl, sda);

    switch (type) {
        case SSD1306_128x64:
            width_ = 128;
            height_ = 64;
            break;
        case SSD1306_128x32:
            width_ = 128;
            height_ = 32;
            break;
    }

    this->num_pages_ = height_ / 8;
}

Display::Display(gpio_num_t scl, gpio_num_t sda, panel_type_t type)
    : Display(scl, sda, type, SSD1306_DEFAULT_ADDRESS) {}

Display::Display() : Display(SSD1306_DEFAULT_PIN_SCL_CLOCK, SSD1306_DEFAULT_PIN_SDA_DATA, SSD1306_DEFAULT_TYPE) {}

void Display::data(uint8_t d) {
    bool ret;
    i2c->start();
    ret = i2c->write(address_);
    if (!ret) {  // NACK
        i2c->stop();
    }
    i2c->write(0x40);  // Co = 0, D/C = 1
    i2c->write(d);
    i2c->stop();
}

void Display::command(uint8_t c) {
    bool ret;
    i2c->start();
    ret = i2c->write(address_);
    if (!ret) {  // NACK
        i2c->stop();
    }

    i2c->write(0x00);  // Co = 0, D/C = 0 (set control and data bit plus 6
                       // following bits to zero)
    i2c->write(c);
    i2c->stop();
}

void Display::command(int c0, int c1, int c2, int c3, int c4, int c5, int c6, int c7, int c8) {
    bool ret;
    i2c->start();
    ret = i2c->write(address_);
    if (!ret) {  // NACK
        i2c->stop();
    }

    i2c->write(0x00);  // Co = 0, D/C = 0

    i2c->write((uint8_t)c0);
    if (c1 >= 0) i2c->write((uint8_t)c1);
    if (c2 >= 0) i2c->write((uint8_t)c2);
    if (c3 >= 0) i2c->write((uint8_t)c3);
    if (c4 >= 0) i2c->write((uint8_t)c4);
    if (c5 >= 0) i2c->write((uint8_t)c5);
    if (c6 >= 0) i2c->write((uint8_t)c6);
    if (c7 >= 0) i2c->write((uint8_t)c7);
    if (c8 >= 0) i2c->write((uint8_t)c8);

    i2c->stop();
}

void Display::command_list(const uint8_t *c, uint8_t n) {
    bool ret;
    i2c->start();
    ret = i2c->write(address_);
    if (!ret) {  // NACK
        i2c->stop();
        return;
    }

    i2c->write(0x00);  // Co = 0, D/C = 0

    uint8_t bytesOut = 1;

    while (n--) {
        if (bytesOut >= MAX_I2C_LIST_LEN) {
            i2c->stop();

            i2c->start();
            ret = i2c->write(address_);
            if (!ret) {  // NACK
                i2c->stop();
                return;
            }

            i2c->write(0x00);  // Co = 0, D/C = 0
            bytesOut = 1;
        }

        i2c->write(*c);
        c++;

        bytesOut++;
    }

    i2c->stop();
}

bool Display::init() {
    term();

    // reset dirty regions
    clear_regions();

    buffer_size_ = width_ * height_ / 8;
    buffer_ = (uint8_t *)malloc(buffer_size_);

    if (buffer_ == nullptr) {
        ESP_LOGE("graphics", "OLED buffer allocation failed.");
        return false;
    }

    // Panel initialization
    // Try send I2C address check if the panel is connected
    i2c->start();
    if (!i2c->write(address_)) {
        i2c->stop();
        ESP_LOGE("graphics", "OLED I2C bus not responding.");

        free(buffer_);
        buffer_ = nullptr;

        return false;
    }
    i2c->stop();

    // Now we assume all sending will be successful
    command(0xae);  // SSD1306_DISPLAYOFF

    int osc_frequency_flags = 0x8; // Oscillator frequency code (RESET = 0b1000/0x8)
    int clock_divide_ratio = 0x0;  // Power-on default from spec (RESET = 0b0000/0x0)
    command(0xd5);                 // SSD1306_SETDISPLAYCLOCKDIV (0x80 default)
    uint8_t display_clock_div = (osc_frequency_flags << 4) | clock_divide_ratio;
    command(display_clock_div);

    float osc_frequency_value = 1000.0f * (float) OSC_FREQUENCY_TABLE[osc_frequency_flags];

    int num_disp_clocks_per_row = 54;  // (2 + 2 + 50)
    int clock_ticks_per_frame = (clock_divide_ratio + 1) * (int) height_ * num_disp_clocks_per_row;
    frequency_ = osc_frequency_value / (float)(clock_ticks_per_frame);

    command(0xa8);         // SSD1306_SETMULTIPLEX
    command(height_ - 1);  // multiplex ratio: 1/64 or 1/32
    command(0xd3);         // SSD1306_SETDISPLAYOFFSET
    command(0x00);         // 0 no display offset (default)
    command(0x40);         // SSD1306_SETSTARTLINE line #0

    if (type_ == SSD1306_128x32) {
        command(0x8d);  // SSD1306_CHARGEPUMP
        command(0x14);  // Charge pump on
    }

    command(0x20);  // SSD1306_MEMORYMODE
    command(0x00);  // 0x0 act like ks0108
    command(0xa1);  // SSD1306_SEGREMAP | 1
    command(0xc8);  // SSD1306_COMSCANDEC
    command(0xda);  // SSD1306_SETCOMPINS
    if (type_ == SSD1306_128x64) {
        command(0x12);
    } else if (type_ == SSD1306_128x32) {
        command(0x02);
    }

    command(0x81);  // SSD1306_SETCONTRAST
    if (type_ == SSD1306_128x64) {
        command(0xcf);
    } else if (type_ == SSD1306_128x32) {
        command(0x2f);
    }

    command(0xd9);  // SSD1306_SETPRECHARGE
    command(0xf1);
    command(0xdb);  // SSD1306_SETVCOMDETECT
    command(0x30);

    if (type_ == SSD1306_128x64) {
        command(0x8d);  // SSD1306_CHARGEPUMP
        command(0x14);  // Charge pump on
    }

    command(0x2e);  // SSD1306_DEACTIVATE_SCROLL
    command(0xa4);  // SSD1306_DISPLAYALLON_RESUME
    command(0xa6);  // SSD1306_NORMALDISPLAY

    clear();
    refresh(true);

    command(0xaf);  // SSD1306_DISPLAYON

    return true;
}

void Display::set_vertical_offset(int ofs) {
    if (ofs < 0) ofs = 0;
    if (ofs >= height_) ofs = height_ - 1;

    command(0x40 | (uint8_t)ofs);
}

float Display::get_frequency() const {
    // calculated from OSC-frequency / (clock-divide-ratio * display-clocks-per-row * multiples-ratio)
    return frequency_;
}

void Display::term() {
    command(0xae);  // SSD_DISPLAYOFF
    command(0x8d);  // SSD1306_CHARGEPUMP
    command(0x10);  // Charge pump off

    if (buffer_) {
        free(buffer_);
        buffer_ = nullptr;
    }
}

uint8_t Display::get_width() { return width_; }

uint8_t Display::get_height() { return height_; }

void Display::clear() {
    switch (type_) {
        case SSD1306_128x64:
            memset(buffer_, 0, 1024);
            break;
        case SSD1306_128x32:
            memset(buffer_, 0, 512);
            break;
    }

    mark_region(0, width_ - 1, 0, height_ - 1);
}

void Display::mark_region(int x, int y) {
    if (!partial_updates_enabled_) return;

    if (y < 0 || y >= height_) return;
    if (x < 0 || x >= width_) return;

    int page = y / 8;
    auto &page_info = page_info_[page];

    if (x < page_info.dirty_left) page_info.dirty_left = x;
    if (x > page_info.dirty_right) page_info.dirty_right = x;
}

void Display::set_page_region(int x_start, int x_end, int page) {
    if (!partial_updates_enabled_) return;

    if (page < 0 || page >= num_pages_) return;

    if (x_start >= width_) return;
    if (x_end < 0 || x_end < x_start) return;

    if (x_start < 0) x_start = 0;
    if (x_end >= width_) x_end = width_ - 1;

    auto &page_info = page_info_[page];

    if (x_start < page_info.dirty_left) page_info.dirty_left = x_start;
    if (x_end > page_info.dirty_right) page_info.dirty_right = x_end;
}

void Display::mark_region(int x_start, int x_end, int y) {
    if (!partial_updates_enabled_) return;

    if (y < 0 || y >= height_) return;

    int page = y / 8;
    set_page_region(x_start, x_end, page);
}

void Display::mark_region(int x_start, int x_end, int y_start, int y_end) {
    if (!partial_updates_enabled_) return;

    if (y_start >= height_) return;
    if (y_end < 0 || y_end < y_start) return;
    if (y_start < 0) y_start = 0;
    if (y_end >= height_) y_end = height_ - 1;

    if (x_start >= width_) return;
    if (x_end < 0 || x_end < x_start) return;
    if (x_start < 0) x_start = 0;
    if (x_end >= width_) x_end = width_ - 1;

    int start_page = y_start / 8;
    int end_page = y_end / 8;

    for (int page = start_page; page <= end_page; page++) {
        set_page_region(x_start, x_end, page);
    }
}

void Display::clear_regions() {
    if (!partial_updates_enabled_) return;

    for (int page = 0; page < num_pages_; page++) {
        // clear horizontal region
        auto &page_info = page_info_[page];
        page_info.dirty_left = 255;  // left
        page_info.dirty_right = 0;   // right
    }
}

void Display::enable_partial_updates(bool enable) {
    partial_updates_enabled_ = enable;
    if (enable) clear_regions();
}

bool Display::is_partial_updates_enabled() const { return partial_updates_enabled_; }

void Display::refresh() { refresh(partial_updates_enabled_ ? false : true); }

void Display::set_page_lock(int page, bool lock) {
    if (page < 0 || page >= num_pages_) return;
    page_info_[page].lock = lock;
}

void Display::refresh(bool force) {
    if (force) {
        command(0x21);            // OPCODE: SSD1306_COLUMNADDR
        command(0);               // column start
        command(width_ - 1);      // column end
        command(0x22);            // OPCODE: SSD1306_PAGEADDR
        command(0);               // page start
        command(num_pages_ - 1);  // page end

        int k = 0;

        while (k < buffer_size_) {
            i2c->start();
            i2c->write(address_);
            i2c->write(0x40);

            int j = 0;
            while (k < buffer_size_ && j < MAX_I2C_LIST_LEN - 1) {
                i2c->write(buffer_[k]);
                ++j;
                ++k;
            }

            i2c->stop();
        }

    } else {
        for (int page = 0; page < num_pages_; page++) {
            refresh_page(page, false);
        }

        // reset dirty area
        clear_regions();
    }
}

void Display::refresh_page(int page, bool force) {
    if (page < 0 || page >= num_pages_) {
        return;
    }

    auto &page_info = page_info_[page];

    // ESP_LOGI("graphics", "region: page %d: %d - %d", page, region.first,
    // region.second);

    if (false == force && (page_info.lock || page_info.dirty_right < page_info.dirty_left)) {
        return;
    }

    // ESP_LOGI("graphics", "draw region/page %d: %d - %d", page, region.first,
    // region.second);

    uint8_t col_start = (uint8_t)page_info.dirty_left;
    uint8_t col_end = (uint8_t)page_info.dirty_right;

    command(0x21);       // SSD1306_COLUMNADDR
    command(col_start);  // column start
    command(col_end);    // column end

    command(0x22);  // SSD1306_PAGEADDR
    command(page);  // page start
    command(page);  // page end

    int page_size = width_;
    int page_offset = page * page_size;
    int write_counter = 0;

    for (int j = col_start; j <= col_end; ++j) {
        if (write_counter == 0) {
            i2c->start();
            i2c->write(address_);
            i2c->write(0x40);
            ++write_counter;
        }

        i2c->write(buffer_[page_offset + j]);
        ++write_counter;

        if (write_counter >= MAX_I2C_LIST_LEN) {
            i2c->stop();
            write_counter = 0;
        }
    }

    if (write_counter != 0) {  // for last batch if stop was not sent
        i2c->stop();
    }

    // clear dirty region
    page_info.dirty_left = 255;
    page_info.dirty_right = 0;
}

void Display::draw_pixel(int8_t x, int8_t y, color_t color) {
    if ((x >= width_) || (x < 0) || (y >= height_) || (y < 0)) return;
    draw_pixel_raw(x, y, color);
    mark_region(x, y);
}

void Display::draw_pixel_raw(int8_t x, int8_t y, color_t color) {

    uint16_t index = x + (y / 8) * width_;
    switch (color) {
        case WHITE:
            buffer_[index] |= (1 << (y & 7));
            break;
        case BLACK:
            buffer_[index] &= ~(1 << (y & 7));
            break;
        case INVERT:
            buffer_[index] ^= (1 << (y & 7));
            break;
        default:
            break;
    }
}

void Display::draw_hline(int8_t x, int8_t y, uint8_t w, color_t color) {
    uint16_t index;
    uint8_t mask, t;

    // check boundaries

    if (w == 0) {
        return;
    }

    if ((x >= width_) || (x + w - 1 < 0) || (y >= height_) || (y < 0)) {
        return;
    }

    if (x < 0) {
        w += x;
        x = 0;
    }

    if (x + w > width_) {
        w = width_ - x;
    }

    t = w;
    index = x + (y / 8) * width_;
    mask = 1 << (y & 7);
    switch (color) {
        case WHITE:
            while (t--) {
                buffer_[index] |= mask;
                ++index;
            }
            break;
        case BLACK:
            mask = ~mask;
            while (t--) {
                buffer_[index] &= mask;
                ++index;
            }
            break;
        case INVERT:
            while (t--) {
                buffer_[index] ^= mask;
                ++index;
            }
            break;
        default:
            break;
    }

    mark_region(x, x + w - 1, y, y);
}

void Display::draw_vline(int8_t x, int8_t y, uint8_t h, color_t color) {
    uint16_t index;
    uint8_t mask, mod, t;

    // boundary check
    if ((x >= width_) || (x < 0) || (y >= height_) || (y < 0)) return;
    if (h == 0) return;
    if (y + h > height_) h = height_ - y;

    t = h;
    index = x + (y / 8) * width_;
    mod = y & 7;

    if (mod) {  // partial line that does not fit into byte at top

        // Magic from Adafruit
        mod = 8 - mod;
        static const uint8_t premask[8] = {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE};
        mask = premask[mod];
        if (t < mod) mask &= (0xFF >> (mod - t));
        switch (color) {
            case WHITE:
                buffer_[index] |= mask;
                break;
            case BLACK:
                buffer_[index] &= ~mask;
                break;
            case INVERT:
                buffer_[index] ^= mask;
                break;
            default:
                break;
        }

        if (t < mod) {
            mark_region(x, x, y, y + h - 1);
            return;
        }

        t -= mod;
        index += width_;
    }

    if (t >= 8) {  // byte aligned line at middle

        switch (color) {
            case WHITE:
                do {
                    buffer_[index] = 0xff;
                    index += width_;
                    t -= 8;
                } while (t >= 8);
                break;
            case BLACK:
                do {
                    buffer_[index] = 0x00;
                    index += width_;
                    t -= 8;
                } while (t >= 8);
                break;
            case INVERT:
                do {
                    buffer_[index] = ~buffer_[index];
                    index += width_;
                    t -= 8;
                } while (t >= 8);
                break;
            default:
                break;
        }
    }
    if (t)  // // partial line at bottom
    {
        mod = t & 7;
        static const uint8_t postmask[8] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F};
        mask = postmask[mod];
        switch (color) {
            case WHITE:
                buffer_[index] |= mask;
                break;
            case BLACK:
                buffer_[index] &= ~mask;
                break;
            case INVERT:
                buffer_[index] ^= mask;
                break;
            default:
                break;
        }
    }

    mark_region(x, x, y, y + h - 1);

    return;
}

void Display::draw_rectangle(int8_t x, int8_t y, uint8_t w, uint8_t h, color_t color) {
    draw_hline(x, y, w, color);
    draw_hline(x, y + h - 1, w, color);
    draw_vline(x, y, h, color);
    draw_vline(x + w - 1, y, h, color);
}

void Display::fill_rectangle(int8_t x, int8_t y, uint8_t w, uint8_t h, color_t color) {
    // Can be optimized?
    uint8_t i;
    for (i = x; i < x + w; ++i) {
        draw_vline(i, y, h, color);
    }
}

void Display::draw_circle(int8_t x0, int8_t y0, uint8_t r, color_t color) {
    // Refer to http://en.wikipedia.org/wiki/Midpoint_circle_algorithm for the
    // algorithm

    int8_t x = r;
    int8_t y = 1;
    int16_t radius_err = 1 - x;

    if (r == 0) return;

    draw_pixel(x0 - r, y0, color);
    draw_pixel(x0 + r, y0, color);
    draw_pixel(x0, y0 - r, color);
    draw_pixel(x0, y0 + r, color);

    while (x >= y) {
        draw_pixel(x0 + x, y0 + y, color);
        draw_pixel(x0 - x, y0 + y, color);
        draw_pixel(x0 + x, y0 - y, color);
        draw_pixel(x0 - x, y0 - y, color);
        if (x != y) {
            /* Otherwise the 4 drawings below are the same as above, causing
             * problem when color is INVERT
             */
            draw_pixel(x0 + y, y0 + x, color);
            draw_pixel(x0 - y, y0 + x, color);
            draw_pixel(x0 + y, y0 - x, color);
            draw_pixel(x0 - y, y0 - x, color);
        }
        ++y;
        if (radius_err < 0) {
            radius_err += 2 * y + 1;
        } else {
            --x;
            radius_err += 2 * (y - x + 1);
        }
    }
}

void Display::fill_circle(int8_t x0, int8_t y0, uint8_t r, color_t color) {
    int8_t x = 1;
    int8_t y = r;
    int16_t radius_err = 1 - y;
    int8_t x1;

    if (r == 0) return;

    draw_vline(x0, y0 - r, 2 * r + 1, color);  // Center vertical line
    while (y >= x) {
        draw_vline(x0 - x, y0 - y, 2 * y + 1, color);
        draw_vline(x0 + x, y0 - y, 2 * y + 1, color);
        if (color != INVERT) {
            draw_vline(x0 - y, y0 - x, 2 * x + 1, color);
            draw_vline(x0 + y, y0 - x, 2 * x + 1, color);
        }
        ++x;
        if (radius_err < 0) {
            radius_err += 2 * x + 1;
        } else {
            --y;
            radius_err += 2 * (x - y + 1);
        }
    }

    if (color == INVERT) {
        x1 = x;  // Save where we stopped

        y = 1;
        x = r;
        radius_err = 1 - x;
        draw_hline(x0 + x1, y0, r - x1 + 1, color);
        draw_hline(x0 - r, y0, r - x1 + 1, color);
        while (x >= y) {
            draw_hline(x0 + x1, y0 - y, x - x1 + 1, color);
            draw_hline(x0 + x1, y0 + y, x - x1 + 1, color);
            draw_hline(x0 - x, y0 - y, x - x1 + 1, color);
            draw_hline(x0 - x, y0 + y, x - x1 + 1, color);
            ++y;
            if (radius_err < 0) {
                radius_err += 2 * y + 1;
            } else {
                --x;
                radius_err += 2 * (y - x + 1);
            }
        }
    }
}

void Display::select_font(uint8_t idx) {
    if (idx >= FONT_COUNT) {
        return;
    }

    font_ = FONT_DATA[idx];
}

void Display::draw_bitmap(const graphics::bitmap_t* bitmap, int x, int y, bool enable_alpha, color_t foreground, color_t background) {

    if (bitmap == nullptr) return;

    auto pixels = bitmap->pixels;
    int height = bitmap->height;
    int width = bitmap->width;
    bool has_alpha = bitmap->alpha_channel;
    int bits_per_pixels = has_alpha ? 2 : 1;
    int bytes_per_line = bitmap->bytes_per_line;

    mark_region(x, x + width - 1, y, y + height - 1);

    for (int j = 0; j < height; ++j) {

        int dest_y = y + j;
        if (dest_y < 0 || dest_y >= height_) continue;
        uint8_t pixel_y = (uint8_t) dest_y;

        uint8_t line = 0x0;
        uint8_t alpha = 0x0;

        for (uint16_t i = 0; i < width; ++i) {

            if (i % 8 == 0) {
                int ofs = j * bytes_per_line + (i / 8) * bits_per_pixels;
                line = pixels[ofs];       // pixel channel
                if (has_alpha) {          // alpha channel
                    alpha = pixels[ofs+1];
                }
            }

            int dest_x = x + i;
            if (dest_x < 0 || dest_x >= width_) {
                line <<= 1;
                if (has_alpha) alpha <<= 1;
                continue;
            }

            uint8_t pixel_x = (uint8_t) dest_x;

            if (has_alpha) {
                if (!enable_alpha || alpha & 0x80) {
                    auto col = (0x0 != (line & 0x80)) ? foreground : background;
                    draw_pixel_raw(pixel_x, pixel_y, col);
                }
                alpha <<= 1;     // shift alpha register, does not matter in case no alpha...
            } else {
                if (line & 0x80) {
                    draw_pixel_raw(pixel_x, pixel_y, foreground);
                } else if (background == WHITE || background == BLACK) {
                    draw_pixel_raw(pixel_x, pixel_y, background);
                }
            }

            line <<= 1;       // shift pixel register
        }
    }
}

void Display::draw_bitmap(const uint8_t *bitmap, int x, int y, int width, int height, color_t foreground, color_t background) {
    if (bitmap == nullptr) return;

    for (uint8_t j = 0; j < height; ++j) {
        uint8_t line = 0x0;

        for (uint8_t i = 0; i < width; ++i) {
            if (i % 8 == 0) {
                line = bitmap[(width + 7) / 8 * j + i / 8];  // line data
            }

            uint8_t pixel_x = (uint8_t)(x + i);
            uint8_t pixel_y = (uint8_t)(y + j);

            if (line & 0x80) {
                draw_pixel(pixel_x, pixel_y, foreground);

            } else {
                switch (background) {
                    case TRANSPARENT:
                        // Not drawing for transparent background
                        break;
                    case WHITE:
                    case BLACK:
                        draw_pixel(pixel_x, pixel_y, background);
                        break;
                    case INVERT:
                        // I don't know why I need invert background
                        break;
                }
            }
            line = line << 1;
        }
    }
}

void Display::draw_masked_bitmap(const uint8_t *bitmap, const uint8_t *mask_bitmap, int x, int y, int width, int height, color_t foreground, color_t background) {
    if (bitmap == nullptr || mask_bitmap == nullptr) return;

    for (uint8_t j = 0; j < height; ++j) {
        uint8_t line = 0x0;
        uint8_t mask = 0x0;

        for (uint8_t i = 0; i < width; ++i) {
            if (i % 8 == 0) {
                int ofs = (width + 7) / 8 * j + i / 8;
                line = bitmap[ofs];       // line data
                mask = mask_bitmap[ofs];  // mask data
            }

            if (mask & 0x80) {
                uint8_t pixel_x = (uint8_t)(x + i);
                uint8_t pixel_y = (uint8_t)(y + j);

                if (line & 0x80) {
                    draw_pixel(pixel_x, pixel_y, foreground);
                } else {
                    switch (background) {
                        case TRANSPARENT:
                            // Not drawing for transparent background
                            break;
                        case WHITE:
                        case BLACK:
                            draw_pixel(pixel_x, pixel_y, background);
                            break;
                        case INVERT:
                            // I don't know why I need invert background
                            break;
                    }
                }
            }

            mask = mask << 1;
            line = line << 1;
        }
    }
}

// return character width
uint8_t Display::draw_char(uint8_t x, uint8_t y, int c, color_t foreground, color_t background) {
    if (font_ == nullptr) {
        return 0;
    }

    if ((c < font_->char_start) || (c > font_->char_end)) {
        c = ' ';  // we always have space in the font set
    }

    size_t char_index = c - font_->char_start;
    const auto &char_descriptor = font_->char_descriptors[char_index];

    const uint8_t *bitmap = font_->bitmap + char_descriptor.offset;
    uint8_t height = font_->height;
    uint8_t width = char_descriptor.width;
    uint8_t line = 0x0;

    for (uint8_t j = 0; j < height; ++j) {
        for (uint8_t i = 0; i < width; ++i) {
            if (i % 8 == 0) {
                int ofs = (width + 7) / 8 * j + i / 8;
                line = bitmap[ofs];
            }

            if (line & 0x80) {
                draw_pixel(x + i, y + j, foreground);
            } else {
                switch (background) {
                    case TRANSPARENT:
                        // not drawing for transparent background
                        break;
                    case WHITE:
                    case BLACK:
                        draw_pixel(x + i, y + j, background);
                        break;
                    case INVERT:
                        // I don't know why I need invert background
                        break;
                }
            }

            line = line << 1;
        }
    }

    return width;
}

uint8_t Display::draw_string(uint8_t x, uint8_t y, const std::string &str, color_t foreground, color_t background) {
    return draw_string(x, y, str.c_str(), foreground, background);
}

uint8_t Display::draw_string(uint8_t x, uint8_t y, const char *str, color_t foreground, color_t background) {
    uint8_t t = x;

    if (font_ == nullptr) {
        return 0;
    }

    if (str == nullptr || *str == '\0') {
        return 0;
    }

    while (*str) {
        x += draw_char(x, y, *str, foreground, background);
        ++str;
        if (*str) x += font_->c;
    }

    return (x - t);
}

// return width of string
uint8_t Display::measure_string(const std::string &str) { return measure_string(str.c_str()); }

// return width of string
uint8_t Display::measure_string(const char *str) {
    if (font_ == nullptr) {
        return 0;
    }

    if (str == nullptr || *str == '\0') {
        return 0;
    }

    uint8_t w = 0;

    while (*str) {
        unsigned char c = *str;
        // we always have space in the font set
        if ((c < font_->char_start) || (c > font_->char_end)) {
            c = ' ';
        }

        c = c - font_->char_start;  // c now become index to tables
        w += font_->char_descriptors[c].width;
        ++str;
        if (*str) {
            w += font_->c;
        }
    }

    return w;
}

uint8_t Display::get_font_height() {
    if (font_ == nullptr) {
        return 0;
    }

    return (font_->height);
}

uint8_t Display::get_font_c() {
    if (font_ == nullptr) return 0;

    return (font_->c);
}

void Display::invert_display(bool invert) {
    if (invert)
        command(0xa7);  // SSD1306_INVERTDISPLAY
    else
        command(0xa6);  // SSD1306_NORMALDISPLAY
}

void Display::update_buffer(uint8_t *data, uint16_t length) {
    if (type_ == SSD1306_128x64) {
        memcpy(buffer_, data, (length < 1024) ? length : 1024);
    } else if (type_ == SSD1306_128x32) {
        memcpy(buffer_, data, (length < 512) ? length : 512);
    }

    mark_region(0, width_ - 1, 0, height_ - 1);
}

void Display::start_scroll_horizontal(int start_page, int end_page, bool right, int time_interval) {
    command(right ? SSD1306_RIGHT_HORIZONTAL_SCROLL : SSD1306_LEFT_HORIZONTAL_SCROLL, 0x0, start_page, time_interval,
            end_page, 0x0, 0xff);

    command(SSD1306_ACTIVATE_SCROLL);
}

void Display::start_scroll(int start_page, int end_page, int start_row, int end_row, bool right, int time_interval, int vertical_offset) {

    int num_rows = end_row - start_row + 1;
    if (num_rows < 0) num_rows = 0;

    if (vertical_offset < 0) {
        vertical_offset = num_rows + vertical_offset;
    }

    command(SSD1306_SET_VERTICAL_SCROLL_AREA, start_row, num_rows,
            right ? SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL : SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL,
            0x0, start_page, time_interval, end_page, vertical_offset);

    command(SSD1306_ACTIVATE_SCROLL);
}

void Display::stop_scroll(void) { command(SSD1306_DEACTIVATE_SCROLL); }
