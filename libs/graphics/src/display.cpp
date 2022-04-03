//
// SSD1306 OLED Display Controller
//
#include "graphics/device.h"
#include "graphics/display.h"

#include <memory.h>
#include <stdint.h>

#include "esp_log.h"
#include "graphics/fonts.h"

using namespace graphics;

// ############################################################################
// Helpers
// ############################################################################

template <typename T> T min(const T& a, const T& b) {
    return (a < b) ? a : b;
}

template <typename T> T max(const T& a, const T& b) {
    return (a < b) ? b : a;
}

template <typename T> void sort_pair(T& a, T& b) {
    if (a > b) {
        T c{a};
        a=b; b=c;
    }
}

// ############################################################################
// Generic methods
// ############################################################################

Display::Display()
    : device_(new Device()) {}

Device* Display::device() {
    return device_;
}

bool Display::init() {
    bool status = device_->init();
    if (false == status) {
        return status;
    }

    setDeferredUpdate(false);       // after application init, it will be enabled
    setBuiltinFont(0);              // select default font 0
    width_ = device_->width();
    height_ = device_->height();

    return true;
}

int Display::width() {
    return width_;
}

int Display::height() {
    return height_;
}

// ############################################################################
// Colors
// ############################################################################

color_t Display::setForeground(color_t foreground) {
    auto old = foreground_;
    foreground_ = foreground;
    return old;
}

color_t Display::getForeground() const {
    return foreground_;
}

color_t Display::setBackground(color_t background) {
    auto old = background_;
    background_ = background;
    return old;
}

color_t Display::getBackground() const {
    return background_;
}

// ############################################################################
// Low-level drawing
// ############################################################################

inline uint8_t Display::getPixelMask(int y) const {
    return (1 << (y & 7));
}

inline uint16_t Display::getPixelOffset(int x, int y) const {
    return (x + (y / 8) * width_);
}

void Display::drawPixelClipped(int x, int y, color_t color) {
    if ((x >= width_) || (x < 0) || (y >= height_) || (y < 0)) return;
    drawPixelRaw(x, y, color);
}

void Display::drawPixelRaw(int x, int y, color_t color) {

    auto buffer = device_->buffer();

    uint8_t dest_bit = getPixelMask(y);
    uint16_t index = getPixelOffset(x, y);
    uint8_t* ptr = buffer + index;
    switch (color) {
        case WHITE:
            *ptr |= dest_bit;
            break;
        case BLACK:
            *ptr &= ~dest_bit;
            break;
        case INVERT:
            *ptr ^= dest_bit;
            break;
        default:
            break;
    }
}

// ############################################################################
// Drawing
// ############################################################################

void Display::clear() {
    device_->clear();
}

void Display::drawPixel(int x, int y) {
    if ((x >= width_) || (x < 0) || (y >= height_) || (y < 0)) return;
    drawPixelRaw(x, y, foreground_);
    device_->markRegion(x, y);
}

void Display::drawPixel(int x, int y, color_t color) {
    if ((x >= width_) || (x < 0) || (y >= height_) || (y < 0)) return;
    drawPixelRaw(x, y, color);
    device_->markRegion(x, y);
}

void Display::drawHorizontalLine(int x, int y, int x2) {
    uint16_t index;
    uint8_t mask, t;

    auto buffer = device_->buffer();

    sort_pair(x, x2);
    uint8_t w = x2 - x + 1;

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
    switch (foreground_) {
        case WHITE:
            while (t--) {
                buffer[index] |= mask;
                ++index;
            }
            break;
        case BLACK:
            mask = ~mask;
            while (t--) {
                buffer[index] &= mask;
                ++index;
            }
            break;
        case INVERT:
            while (t--) {
                buffer[index] ^= mask;
                ++index;
            }
            break;
        default:
            break;
    }

    device_->markRegion(x, x + w - 1, y, y);
}

void Display::drawVerticalLine(int x, int y, int y2) {
    int index;
    uint8_t mask, mod, t;

    auto buffer = device_->buffer();

    sort_pair(y, y2);
    int h = y2 - y + 1;

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
        switch (foreground_) {
            case WHITE:
                buffer[index] |= mask;
                break;
            case BLACK:
                buffer[index] &= ~mask;
                break;
            case INVERT:
                buffer[index] ^= mask;
                break;
            default:
                break;
        }

        if (t < mod) {
            device_->markRegion(x, x, y, y + h - 1);
            return;
        }

        t -= mod;
        index += width_;
    }

    if (t >= 8) {  // byte aligned line at middle
        switch (foreground_) {
            case WHITE:
                do {
                    buffer[index] = 0xff;
                    index += width_;
                    t -= 8;
                } while (t >= 8);
                break;
            case BLACK:
                do {
                    buffer[index] = 0x00;
                    index += width_;
                    t -= 8;
                } while (t >= 8);
                break;
            case INVERT:
                do {
                    buffer[index] = ~buffer[index];
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
        switch (foreground_) {
            case WHITE:
                buffer[index] |= mask;
                break;
            case BLACK:
                buffer[index] &= ~mask;
                break;
            case INVERT:
                buffer[index] ^= mask;
                break;
            default:
                break;
        }
    }

    device_->markRegion(x, x, y, y + h - 1);

    return;
}


void Display::drawLine(int x, int y, int x2, int y2) {
    if (x == x2) {
        drawVerticalLine(x, y, y2);
        return;
    }

    if (y == y2) {
        drawHorizontalLine(x, y, x2);
        return;
    }

    device_->markRegion(min(x, x2), max(x, x2), min(y, y2), max(y, y2));

    bool vertical = false;
    int short_length = y2 - y;
    int long_length = x2 - x;

    if (abs(short_length) > abs(long_length)) {
        int swap = short_length;
        short_length = long_length;
        long_length = swap;
        vertical = true;
    }

    int dec_inc = (long_length != 0) ? (short_length << 16) / long_length : 0;
    bool do_increment = (long_length > 0);

    auto color = foreground_;

    if (vertical) {
        long_length += y;
        if (do_increment) {
            for (int j = 0x8000 + (x << 16); y <= long_length; ++y) {
                drawPixelClipped(j >> 16, y, color);
                j += dec_inc;
            }
        } else {
            for (int j = 0x8000 + (x << 16); y >= long_length; --y) {
                drawPixelClipped(j >> 16, y, color);
                j -= dec_inc;
            }
        }
    } else {
        long_length += x;
        if (do_increment) {
            for (int j = 0x8000 + (y << 16); x <= long_length; ++x) {
                drawPixelClipped(x, j >> 16, color);
                j += dec_inc;
            }
        } else {
            for (int j = 0x8000 + (y << 16); x >= long_length; --x) {
                drawPixelClipped(x, j >> 16, color);
                j -= dec_inc;
            }
        }
    }
}

void Display::drawRectangle(int x, int y, int x2, int y2) {
    sort_pair(x, x2);
    sort_pair(y, y2);
    drawHorizontalLine(x, y, x2);
    drawHorizontalLine(x, y2, x2);
    drawVerticalLine(x, y, y2);
    drawVerticalLine(x2, y, y2);
}

void Display::fillRectangle(int x, int y, int x2, int y2) {
    // Can be optimized?
    sort_pair(x, x2);
    sort_pair(y, y2);
    for (auto i = x; i <= x2; ++i) {
        drawVerticalLine(i, y, y2);
    }
}

void Display::drawCircle(int x0, int y0, int r) {
    // Refer to http://en.wikipedia.org/wiki/Midpoint_circle_algorithm for the
    // algorithm

    int x = r;
    int y = 1;
    int radius_err = 1 - x;

    if (r == 0) return;

    drawPixel(x0 - r, y0);
    drawPixel(x0 + r, y0);
    drawPixel(x0, y0 - r);
    drawPixel(x0, y0 + r);

    while (x >= y) {
        drawPixel(x0 + x, y0 + y);
        drawPixel(x0 - x, y0 + y);
        drawPixel(x0 + x, y0 - y);
        drawPixel(x0 - x, y0 - y);
        if (x != y) {
            /* Otherwise the 4 drawings below are the same as above, causing
             * problem when color is INVERT
             */
            drawPixel(x0 + y, y0 + x);
            drawPixel(x0 - y, y0 + x);
            drawPixel(x0 + y, y0 - x);
            drawPixel(x0 - y, y0 - x);
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

void Display::fillCircle(int x0, int y0, int r) {
    int x = 1;
    int y = r;
    int radius_err = 1 - y;
    int x1;

    if (r == 0) return;

    drawVerticalLine(x0, y0 - r, y0 + r);  // Center vertical line
    while (y >= x) {
        drawVerticalLine(x0 - x, y0 - y, y0 + y);
        drawVerticalLine(x0 + x, y0 - y, y0 + y);
        if (foreground_ != INVERT) {
            drawVerticalLine(x0 - y, y0 - x, y0 + x);
            drawVerticalLine(x0 + y, y0 - x, y0 + x);
        }
        ++x;
        if (radius_err < 0) {
            radius_err += 2 * x + 1;
        } else {
            --y;
            radius_err += 2 * (x - y + 1);
        }
    }

    if (foreground_ == INVERT) {

        x1 = x;  // Save where we stopped

        y = 1;
        x = r;
        radius_err = 1 - x;
        drawHorizontalLine(x0 + x1, y0, x0 + r);
        drawHorizontalLine(x0 - r, y0, x0 - x1);
        while (x >= y) {
            drawHorizontalLine(x0 + x1, y0 - y, x0 + x);
            drawHorizontalLine(x0 + x1, y0 + y, x0 + x);
            drawHorizontalLine(x0 - x, y0 - y, x0 - x1);
            drawHorizontalLine(x0 - x, y0 + y, x0 - x1);
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

// ############################################################################
// Text and Fonts
// ############################################################################

const font_t* Display::font() const {
    return font_;
}

const font_t* Display::setFont(const font_t* font) {
    auto old_font = font_;
    font_ = font;
    return old_font;
}

const font_t* Display::setBuiltinFont(uint8_t idx) {
    if (idx < BUILTIN_FONT_COUNT) {
        return setFont(BUILTIN_FONTS[idx]);
    }

    return font_;
}

// return character width
int Display::drawChar(int x, int y, int c) {
    if (font_ == nullptr) {
        return 0;
    }

    if ((c < font_->char_start) || (c > font_->char_end)) {
        c = ' ';  // we always have space in the font set
    }

    size_t char_index = c - font_->char_start;
    const auto &char_descriptor = font_->char_descriptors[char_index];

    const uint8_t *bitmap = font_->bitmap + char_descriptor.offset;
    int height = font_->height;
    int width = char_descriptor.width;
    uint8_t line = 0x0;

    for (uint8_t j = 0; j < height; ++j) {
        for (uint8_t i = 0; i < width; ++i) {
            if (i % 8 == 0) {
                int ofs = (width + 7) / 8 * j + i / 8;
                line = bitmap[ofs];
            }

            if (line & 0x80) {
                drawPixel(x + i, y + j, foreground_);
            } else {
                switch (background_) {
                    case TRANSPARENT:
                        // not drawing for transparent background
                        break;
                    case WHITE:
                    case BLACK:
                        drawPixel(x + i, y + j, background_);
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

int Display::drawString(int x, int y, const std::string &str) {
    return drawString(x, y, str.c_str());
}

int Display::drawString(int x, int y, const char *str) {
    int t = x;

    if (font_ == nullptr) {
        return 0;
    }

    if (str == nullptr || *str == '\0') {
        return 0;
    }

    while (*str) {
        x += drawChar(x, y, *str);
        ++str;
        if (*str) x += font_->c;
    }

    return (x - t);
}

// return width of string
int Display::measureString(const std::string &str) {
    return measureString(str.c_str());
}

// return width of string
int Display::measureString(const char *str) {
    if (font_ == nullptr) {
        return 0;
    }

    if (str == nullptr || *str == '\0') {
        return 0;
    }

    int w = 0;

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

// ############################################################################
// Bitmaps
// ############################################################################

void Display::drawBitmap(const graphics::bitmap_t* bitmap,
                         int x, int y,
                         bool enable_alpha) {

    if (bitmap == nullptr) return;

    auto buffer = device_->buffer();

    auto pixels = bitmap->pixels;
    int height = bitmap->height;
    int width = bitmap->width;
    bool has_alpha = bitmap->alpha_channel;
    int bits_per_pixels = has_alpha ? 2 : 1;
    int bytes_per_line = bitmap->bytes_per_line;

    device_->markRegion(x, x + width - 1, y, y + height - 1);

    int line_offset = -bytes_per_line; // next inc leads to zero

    for (int j = 0; j < height; ++j) {
        line_offset += bytes_per_line;

        int dest_y = y + j;
        if (dest_y < 0 || dest_y >= height_) continue;
        uint8_t pixel_y = (uint8_t) dest_y;
        uint8_t dest_bit = (1 << (pixel_y & 7));
        int dest_line_index = (pixel_y / 8) * width_;

        uint8_t line = 0x0;
        uint8_t alpha = 0x0;

        int ofs = line_offset - bits_per_pixels;

        for (int i = 0; i < width; ++i) {

            if (i % 8 == 0) {
                ofs += bits_per_pixels;
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
            int dest_index = dest_line_index + pixel_x;
            uint8_t* dest_ptr = buffer + dest_index;

            if (has_alpha) {
                if (!enable_alpha || alpha & 0x80) {
                    if (line & 0x80) {
                        *dest_ptr |= dest_bit;
                    } else {
                        *dest_ptr &= ~dest_bit;
                    }
                }
                alpha <<= 1;     // shift alpha register, does not matter in case no alpha...
            } else {
                if (line & 0x80) {
                    *dest_ptr |= dest_bit;
                } else if (background_ == WHITE || background_ == BLACK) {
                    *dest_ptr &= ~dest_bit;
                }
            }
            line <<= 1;       // shift pixel register
        }
    }
}

// ############################################################################
// Scrolling
// ############################################################################

void Display::startHorizontalScrolling(int start_page, int end_page, bool right, int time_interval) {
    device_->startHorizontalScrolling(start_page, end_page, right, time_interval);
}

void Display::startDiagonalScrolling(int start_page, int end_page, int start_row, int end_row,
                                     bool right, int time_interval, int vertical_offset) {
    device_->startDiagonalScrolling(start_page, end_page, start_row, end_row, right,
                                    time_interval, vertical_offset);
}

void Display::stopScrolling(void) {
    device_->stopScrolling();
}

// ############################################################################
// Display update
// ############################################################################

void Display::update() {
    if (deferred_update_) {
        setUpdateState(UPDATE_NEEDED);
    } else {
        device_->refresh();
        update_state_ = NO_UPDATE_NEEDED;
    }
}

void Display::update(bool force) {
    if (deferred_update_) {
        setUpdateState(force ? FORCED_UPDATE : UPDATE_NEEDED);
    } else {
        device_->refresh(force);
        update_state_ = NO_UPDATE_NEEDED;
    }
}

void Display::refresh() {
    if (update_state_ == UPDATE_NEEDED) {
        device_->refresh();
    } else if (update_state_ == FORCED_UPDATE) {
        device_->refresh(true);
    }
    update_state_ = NO_UPDATE_NEEDED;
}

void Display::setPartialUpdate(bool enable) {
    device_->enablePartialUpdates(enable);
}
bool Display::getPartialUpdate() const {
    return device_->isPartialUpdatesEnabled();
}

void Display::setDeferredUpdate(bool deferred_update) {
    deferred_update_ = deferred_update;
}
bool Display::getDeferredUpdate() const {
    return deferred_update_;
}

void Display::setUpdateState(update_state_t update_state) {
    update_state_ = update_state;
}

Display::update_state_t Display::getUpdateState() const {
    return update_state_;
}

void Display::lockPage(int page, bool lock) {
    device_->lockPage(page, lock);
}
