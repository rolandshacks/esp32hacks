//
// SSD1306 OLED Device Controller
//
#include "graphics/device.h"

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

Device::Device(gpio_num_t scl, gpio_num_t sda, panel_type_t type, uint8_t address)
    : i2c(nullptr),
      type_(type),
      address_(address),
      buffer_(nullptr),
      buffer_size_(0),
      width_(0),
      height_(0),
      num_pages_(0),
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

Device::Device(gpio_num_t scl, gpio_num_t sda, panel_type_t type)
    : Device(scl, sda, type, SSD1306_DEFAULT_ADDRESS) {}

Device::Device() : Device(SSD1306_DEFAULT_PIN_SCL_CLOCK, SSD1306_DEFAULT_PIN_SDA_DATA, SSD1306_DEFAULT_TYPE) {}

void Device::data(uint8_t d) {
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

void Device::command(uint8_t c) {
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

void Device::command(int c0, int c1, int c2, int c3, int c4, int c5, int c6, int c7, int c8) {
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

void Device::command_list(const uint8_t *c, uint8_t n) {
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

bool Device::init() {
    term();

    // reset dirty regions
    clearRegions();

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

void Device::setVerticalOffset(int ofs) {
    if (ofs < 0) ofs = 0;
    if (ofs >= height_) ofs = height_ - 1;

    command(0x40 | (uint8_t)ofs);
}

float Device::frequency() const {
    // calculated from OSC-frequency / (clock-divide-ratio * display-clocks-per-row * multiples-ratio)
    return frequency_;
}

void Device::term() {
    command(0xae);  // SSD_DISPLAYOFF
    command(0x8d);  // SSD1306_CHARGEPUMP
    command(0x10);  // Charge pump off

    if (buffer_) {
        free(buffer_);
        buffer_ = nullptr;
    }
}

uint8_t Device::width() { return width_; }

uint8_t Device::height() { return height_; }

void Device::clear() {
    switch (type_) {
        case SSD1306_128x64:
            memset(buffer_, 0, 1024);
            break;
        case SSD1306_128x32:
            memset(buffer_, 0, 512);
            break;
    }

    markRegion(0, width_ - 1, 0, height_ - 1);
}

void Device::setPageRegion(int x_start, int x_end, int page) {
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

void Device::markRegion(int x, int y) {
    if (!partial_updates_enabled_) return;

    if (y < 0 || y >= height_) return;
    if (x < 0 || x >= width_) return;

    int page = y / 8;
    auto &page_info = page_info_[page];

    if (x < page_info.dirty_left) page_info.dirty_left = x;
    if (x > page_info.dirty_right) page_info.dirty_right = x;
}

void Device::markRegion(int x_start, int x_end, int y) {
    if (!partial_updates_enabled_) return;

    if (y < 0 || y >= height_) return;

    int page = y / 8;
    setPageRegion(x_start, x_end, page);
}

void Device::markRegion(int x_start, int x_end, int y_start, int y_end) {
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
        setPageRegion(x_start, x_end, page);
    }
}

void Device::clearRegions() {
    if (!partial_updates_enabled_) return;

    for (int page = 0; page < num_pages_; page++) {
        // clear horizontal region
        auto &page_info = page_info_[page];
        page_info.dirty_left = 255;  // left
        page_info.dirty_right = 0;   // right
    }
}

void Device::invertDisplay(bool invert) {
    if (invert)
        command(0xa7);  // SSD1306_INVERTDISPLAY
    else
        command(0xa6);  // SSD1306_NORMALDISPLAY
}

void Device::updateBuffer(uint8_t *data, uint16_t length) {
    if (type_ == SSD1306_128x64) {
        memcpy(buffer_, data, (length < 1024) ? length : 1024);
    } else if (type_ == SSD1306_128x32) {
        memcpy(buffer_, data, (length < 512) ? length : 512);
    }

    markRegion(0, width_ - 1, 0, height_ - 1);
}

void Device::enablePartialUpdates(bool enable) {
    partial_updates_enabled_ = enable;
    if (enable) clearRegions();
}

bool Device::isPartialUpdatesEnabled() const {
    return partial_updates_enabled_;
}

void Device::lockPage(int page, bool lock) {
    if (page < 0 || page >= num_pages_) return;
    page_info_[page].lock = lock;
}

void Device::refresh() {
    refresh(partial_updates_enabled_ ? false : true);
}

void Device::refresh(bool force) {
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
            refreshPage(page, false);
        }

        // reset dirty area
        clearRegions();
    }
}

void Device::refreshPage(int page, bool force) {
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

void Device::startHorizontalScrolling(int start_page, int end_page, bool right, int time_interval) {
    stopScrolling(); // make sure scrolling is disabled

    command(right ? SSD1306_RIGHT_HORIZONTAL_SCROLL : SSD1306_LEFT_HORIZONTAL_SCROLL, 0x0, start_page, time_interval,
            end_page, 0x0, 0xff);

    command(SSD1306_ACTIVATE_SCROLL);
}

void Device::startDiagonalScrolling(int start_page, int end_page, int start_row, int end_row, bool right, int time_interval, int vertical_offset) {
    stopScrolling(); // make sure scrolling is disabled

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

void Device::stopScrolling(void) {
    command(SSD1306_DEACTIVATE_SCROLL);
}

uint8_t* Device::buffer() {
    return buffer_;
}

const uint8_t* Device::buffer() const {
    return buffer_;
}

uint16_t Device::bufferSize() const {
    return buffer_size_;
}
