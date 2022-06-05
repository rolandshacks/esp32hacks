//
// Display
//
#include "graphics/base.h"
#include "graphics/bitmap.h"
#include "graphics/display.h"

#include <memory.h>
#include <stdint.h>
#include <algorithm>

#include "esp_log.h"

using namespace graphics;

// ############################################################################
// Helpers
// ############################################################################

static const uint8_t dither_seed[] = { // pre-randomize 256 8-bit integers
    203,22,77,162,177,233,222,231,130,85,19,117,28,206,23,200,118,217,29,207,
    138,41,174,201,224,52,235,133,208,108,11,168,226,199,91,99,123,170,160,1,
    83,111,150,102,161,8,127,53,13,253,164,70,191,244,4,49,68,135,112,82,96,
    128,27,64,35,151,140,247,234,227,209,66,51,129,169,71,182,107,189,42,89,
    192,69,17,60,149,213,184,57,167,12,95,79,0,47,219,134,251,113,242,195,21,
    45,147,54,121,171,176,109,10,202,126,14,50,6,122,146,241,238,30,198,196,72,
    211,90,194,225,58,61,142,59,239,31,158,103,76,143,136,152,73,9,65,155,188,
    116,145,74,120,166,157,16,24,94,144,119,163,255,100,175,228,63,78,250,36,44,
    173,104,179,221,88,232,114,204,220,141,86,5,67,80,110,214,40,187,223,252,26,
    7,125,218,115,3,230,248,38,240,181,180,137,39,92,186,75,159,154,156,34,178,
    148,37,215,246,131,55,105,15,245,62,81,243,172,193,237,25,33,212,98,216,32,
    236,46,183,197,18,185,249,101,48,210,20,87,124,43,106,93,132,153,139,84,205,
    254,229,56,165,97,190,2
};

static const uint8_t bayer_matrix[] = { // 4x4 bayer matrix for 16 palette entries
    0, 8, 2, 10,
    12, 4, 14, 6,
    3, 11, 1, 9,
    15, 7, 13, 5
};

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

Color Display::setForeground(Color foreground) {
    auto old = foreground_;
    foreground_ = foreground;
    return old;
}

Color Display::getForeground() const {
    return foreground_;
}

Color Display::setBackground(Color background) {
    auto old = background_;
    background_ = background;
    return old;
}

Color Display::getBackground() const {
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

void Display::drawPixelClipped(int x, int y, Color color) {
    if ((x >= width_) || (x < 0) || (y >= height_) || (y < 0)) return;
    drawPixelRaw(x, y, color);
}

void Display::drawPixelRaw(int x, int y, Color color) {

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

void Display::drawPixel(int x, int y, Color color) {
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
                buffer[index] &= ~mask;
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

    device_->markRegion(std::min(x, x2), std::max(x, x2), std::min(y, y2), std::max(y, y2));

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

namespace graphics {
    // external access to built-in fonts
    extern const Font* BUILTIN_FONTS[];
    extern const size_t BUILTIN_FONT_COUNT;
}

const Font* Display::font() const {
    return font_;
}

const Font* Display::setFont(const Font* font) {
    auto old_font = font_;
    font_ = font;
    return old_font;
}

const Font* Display::setBuiltinFont(uint8_t idx) {
    if (idx < graphics::BUILTIN_FONT_COUNT) {
        return setFont(graphics::BUILTIN_FONTS[idx]);
    }

    return font_;
}

const FontCharDescriptor* Display::getFontCharDescriptor(char c) const {
    if (font_ == nullptr) {
        return nullptr;
    }

    if ((c < font_->char_start) || (c > font_->char_end)) {
        c = ' ';  // we always have space in the font set
    }

    size_t char_index = c - font_->char_start;
    return &font_->char_descriptors[char_index];
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

static inline void apply_pixel_op(uint8_t* pixel, uint8_t dest_bit, Color color_op) {
    if (color_op == WHITE) {
        *pixel |= dest_bit;
    } else if (color_op == BLACK) {
        *pixel &= ~dest_bit;
    } else if (color_op == INVERT) {
        *pixel ^= dest_bit;
    }
}

void Display::drawBitmap(const Bitmap* bitmap,
                         int x, int y,
                         bool enable_alpha) {
    if (bitmap == nullptr) return;
    drawBitmap(bitmap, nullptr, x, y, enable_alpha);
}

void Display::drawBitmap(const Bitmap* bitmap,
                         const Rectangle* src_rect,
                         int x, int y,
                         bool enable_alpha) {

    if (bitmap == nullptr) return;

    auto buffer = device_->buffer();
    auto pixels = bitmap->getPixelBytes();
    int height = bitmap->height();
    int width = bitmap->width();
    bool has_alpha = bitmap->hasAlpha();
    int bits_per_pixels = has_alpha ? 2 : 1;
    int bytes_per_line = bitmap->bytesPerLine();

    int x1, y1, x2, y2;
    if (nullptr != src_rect) {
        x1 = src_rect->left;
        if (x1 < 0 || x1 >= width) return;
        x2 = src_rect->right;
        if (x2 < x1 || x2 >= width) return;
        y1 = src_rect->top;
        if (y1 < 0 || y1 >= height) return;
        y2 = src_rect->bottom;
        if (y2 < y1 || y2 >= height) return;
    } else {
        x1 = 0;
        y1 = 0;
        x2 = width - 1;
        y2 = height - 1;
    }

    int y_src_min = std::max(y1, -y);
    if (y_src_min >= height) return; // invisible

    int y_src_max = std::min(y2, height_-y-1);
    if (y_src_max < y_src_min) return; // invisible

    int x_src_min = std::max(x1, -x);
    if (x_src_min >= width) return; // invisible

    int x_src_max = std::min(x2, width_-x-1);
    if (x_src_max < x_src_min) return; // invisible

    if (!has_alpha) enable_alpha = false;

    if (y < 0) y = 0;
    if (x < 0) x = 0;

    device_->markRegion(x, x + width - 1, y, y + height - 1);

    int dest_y = y;
    for (int j = y_src_min; j <= y_src_max; ++j) {

        if (dest_y < 0 ||dest_y >= height_) {
            break;
        }

        int line_offset = j * bytes_per_line;

        uint8_t pixel_y = (uint8_t) dest_y;
        uint8_t dest_bit = (1 << (pixel_y & 7));
        int dest_line_index = (pixel_y / 8) * width_;

        uint8_t line = 0x0;
        uint8_t alpha = 0x0;

        int ofs = line_offset + (x_src_min / 8) * bits_per_pixels;

        int dest_x = x;

        for (int i = x_src_min; i <= x_src_max; ++i) {

            int bit = (i%8);

            if (0 == bit || i == x_src_min) {                   // fetch source pixel
                line = (pixels[ofs] << bit);                    // read byte from pixel channel
                if (has_alpha) {
                    alpha = (pixels[ofs+1] << bit);             // read byte from alpha channel
                }
                ofs += bits_per_pixels;                         // advance offset
            }

            if (dest_x >= 0 || dest_x < width_) {               // do horizontal clipping
                int dest_index = dest_line_index + dest_x;
                uint8_t* dest_ptr = buffer + dest_index;

                if (!enable_alpha || alpha & 0x80) {
                    bool bit_is_set = (0x0 != (line & 0x80));
                    apply_pixel_op(dest_ptr, dest_bit, bit_is_set ? foreground_ : background_);
                }
            }

            line <<= 1;                                         // shift pixel register
            alpha <<= 1;                                        // shift alpha register (even if disabled)

            dest_x++;                                           // next pixel
        }

        dest_y++;
    }
}


void Display::drawStretchBitmap(const Bitmap* bitmap,
                                const Rectangle& src_rect,
                                const Rectangle& dest_rect,
                                bool enable_alpha) {

    if (bitmap == nullptr) return;

    auto buffer = device_->buffer();
    auto pixels = bitmap->getPixelBytes();
    bool has_alpha = bitmap->hasAlpha();
    int bits_per_pixels = has_alpha ? 2 : 1;
    int bytes_per_line = bitmap->bytesPerLine();

    Rectangle clipped_dest = dest_rect;
    clipped_dest.clip(0, width()-1, 0, height()-1);

    device_->markRegion(clipped_dest);

    auto src_left = src_rect.left;
    auto src_top = src_rect.top;

    auto src_width = src_rect.width();
    auto src_height = src_rect.height();
    if (src_width < 1 || src_height < 1) return;

    auto dest_width = dest_rect.width();
    auto dest_height = dest_rect.height();
    if (dest_width < 1 || dest_height < 1) return;

    for (int dest_y = clipped_dest.top; dest_y < clipped_dest.bottom; dest_y++) {

        int dest_y_ofs = dest_y - dest_rect.top;
        int src_y = src_top + dest_y_ofs * src_height / dest_height;

        if (src_y < 0 || src_y >= bitmap->height()) continue;

        int dest_line_offset = getPixelOffset(0, dest_y);
        uint8_t dest_mask = getPixelMask(dest_y);

        int src_line_offset = src_y * bytes_per_line;

        for (int dest_x = clipped_dest.left; dest_x < clipped_dest.right; dest_x++) {

            int dest_x_ofs = dest_x - dest_rect.left;
            int src_x = src_left + dest_x_ofs * src_width / dest_width;
            if (src_x < 0 || src_x >= bitmap->width()) continue;

            int src_offset = src_line_offset + (src_x / 8) * bits_per_pixels;
            int src_mask = 1 << (7-(src_x%8));

            Color col = (pixels[src_offset] & src_mask) != 0 ? foreground_ : background_;
            bool alpha = true;

            if (enable_alpha) alpha = (pixels[src_offset+1] & src_mask) != 0 ? true : false;

            if (alpha) {
                int dest_index = dest_line_offset + dest_x;
                uint8_t* dest_ptr = buffer + dest_index;
                apply_pixel_op(dest_ptr, dest_mask, col);
            }
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

// ############################################################################
// Advanced rendering support
// ############################################################################

void Display::enableUnorderedDithering(bool enable) {
    unordered_dithering_ = enable;
}

int Display::getDitheredColor(int x, int y, int intensity) {

    if (intensity == 0) return 0;
    if (intensity >= 255) return 1;

    int r;

    if (unordered_dithering_) {
        // unordered dithering
        r =  dither_seed[((x + 13) * (y + 17))%256];
    } else {
        // ordered dithering
        r = bayer_matrix[((y&0x3)<<2) + (x&0x3)] << 4;
    }


    return (intensity >= r) ? 1 : 0;
}

void Display::drawDitheredHorizontalLine(int x, int y, int x2, int intensity) {
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

    while (t--) {
        auto col = getDitheredColor(x, y, intensity) ? Color::WHITE : Color::BLACK;
        if (0 != col)
            buffer[index] |= mask;
        else
            buffer[index] &= ~mask;

        ++index;
        ++x;
    }

    device_->markRegion(x, x + w - 1, y, y);
}


void Display::drawPatternHorizontalLine(int x, int y, int x2, uint32_t pattern) {

    if (0x0 == pattern) {
        return;
    }

    if (0xffffffff == pattern) {
        drawHorizontalLine(x, y, x2);
        return;
    }

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

    int counter = 0;

    while (t--) {
        uint32_t col = pattern & (1 << (31-(counter % 32)));
        if (0 != col)
            buffer[index] |= mask;
        else
            buffer[index] &= ~mask;

        ++counter;
        ++index;
        ++x;
    }

    device_->markRegion(x, x + w - 1, y, y);
}

void Display::fillDitheredRectangle(int x, int y, int x2, int y2, int intensity) {
    // Can be optimized?
    sort_pair(x, x2);
    sort_pair(y, y2);
    for (auto i = y; i <= y2; ++i) {
        drawDitheredHorizontalLine(x, i, x2, intensity);
    }
}

void Display::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3) {
    drawLine(x1, y1, x2, y2);
    drawLine(x2, y2, x3, y3);
    drawLine(x3, y3, x1, y1);
}

void Display::fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3) {

    auto h = height();

    // sort
    if (y1 > y2) { std::swap(x1, x2); std::swap(y1, y2); }
    if (y1 > y3) { std::swap(x1, x3); std::swap(y1, y3); }
    if (y2 > y3) { std::swap(x2, x3); std::swap(y2, y3); }

    int total_height = (y3 - y1) + 1;

    int last_min = -1;
    int last_max = 128;

    int ofs = (y1 >= 0) ? 0 : -y1;

    for (int i = ofs; i < total_height; i++) {

        bool second_half = (i > y2 - y1) || (y2 == y1);
        int segment_height = second_half ? y3 - y2 : y2 - y1;
        if (segment_height < 1) continue;

        float alpha = (float) i / (float) total_height;
        float beta  = (float) (i - (second_half ? (y2-y1) : 0)) / (float) segment_height;
        int ax = x1 + (x3 - x1) * alpha;
        int bx = second_half ? x2 + (x3 - x2) * beta : x1 + (x2 - x1) * beta;
        if (ax > bx) { std::swap(ax, bx); }

        // avoid gaps
        if (ax > last_max) ax = last_max;
        if (bx < last_min) bx = last_min;

        // draw
        drawHorizontalLine(ax, y1 + i, bx);

        if (y1 + i >= h) break;

        last_min = ax; last_max = bx;
    }
}

void Display::fillDitheredTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int intensity) {

    auto h = height();

    // sort
    if (y1 > y2) { std::swap(x1, x2); std::swap(y1, y2); }
    if (y1 > y3) { std::swap(x1, x3); std::swap(y1, y3); }
    if (y2 > y3) { std::swap(x2, x3); std::swap(y2, y3); }

    int total_height = (y3 - y1) + 1;

    int last_min = -1;
    int last_max = 128;

    int ofs = (y1 >= 0) ? 0 : -y1;

    for (int i = ofs; i < total_height; i++) {

        bool second_half = (i > y2 - y1) || (y2 == y1);
        int segment_height = second_half ? y3 - y2 : y2 - y1;
        if (segment_height < 1) continue;

        float alpha = (float) i / (float) total_height;
        float beta  = (float) (i - (second_half ? (y2-y1) : 0)) / (float) segment_height;
        int ax = x1 + (x3 - x1) * alpha;
        int bx = second_half ? x2 + (x3 - x2) * beta : x1 + (x2 - x1) * beta;
        if (ax > bx) { std::swap(ax, bx); }

        // avoid gaps
        if (ax > last_max) ax = last_max;
        if (bx < last_min) bx = last_min;

        // draw
        drawDitheredHorizontalLine(ax, y1 + i, bx, intensity);

        if (y1 + i >= h) break;

        last_min = ax; last_max = bx;
    }
}
