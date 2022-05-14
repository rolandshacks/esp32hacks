//
// SSD1306 OLED Device Controller
//
#include "graphics/device.h"

#include <memory.h>
#include <stdint.h>

#include "esp_log.h"
#include "graphics/bits.h"

static const uint8_t DEFAULT_GPIO_ADDRESS = 0x78;
static const int DEFAULT_GPIO_PIN_SDA_DATA = 21;
static const int DEFAULT_GPIO_PIN_SCL_CLOCK = 22;
static const graphics::PanelType DEFAULT_PANEL_TYPE = graphics::PanelType::SSD1306_128x64;

/// Oscillator frequency table for 0xD5 command
static const uint16_t OSC_FREQUENCY_TABLE[] = {270, 279, 289, 298, 314, 326, 337, 352,
                                               372, 391, 409, 431, 451, 477, 506, 536};

using namespace graphics;

Device::Device(int scl, int sda, PanelType type, uint8_t address)
    : i2c(nullptr),
      type_(type),
      buffer_(nullptr),
      buffer_size_(0),
      width_(0),
      height_(0),
      num_pages_(0),
      partial_updates_enabled_(true),
      frequency_(0.0f) {

    i2c = new sys::I2C(scl, sda, address);

    switch (type) {
        case PanelType::SSD1306_128x64:
            width_ = 128;
            height_ = 64;
            break;
        case PanelType::SSD1306_128x32:
            width_ = 128;
            height_ = 32;
            break;
    }

    this->num_pages_ = height_ / 8;
}

Device::Device(int scl, int sda, PanelType type)
    : Device(scl, sda, type, DEFAULT_GPIO_ADDRESS) {}

Device::Device() : Device(DEFAULT_GPIO_PIN_SCL_CLOCK, DEFAULT_GPIO_PIN_SDA_DATA, DEFAULT_PANEL_TYPE) {}

void Device::data(uint8_t d) {
    i2c->sendData(d);
}

void Device::command(Command c) {
    command(static_cast<uint8_t>(c));
}

void Device::command(uint8_t c) {
    i2c->sendControl(c);
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

    // Try send I2C address check if the panel is connected
    bool devicePresent = i2c->sendEmpty();
    if (!devicePresent) {
        ESP_LOGE("graphics", "OLED I2C bus not responding.");
        free(buffer_);
        buffer_ = nullptr;
        return false;
    }

    // Now we assume all sending will be successful
    command(Command::SetDisplayOff);

    int osc_frequency_flags = 0x8; // Oscillator frequency code (RESET = 0b1000/0x8)
    int clock_divide_ratio = 0x0;  // Power-on default from spec (RESET = 0b0000/0x0)
    command(Command::SetDisplayClockDivider);
    uint8_t display_clock_div = (osc_frequency_flags << 4) | clock_divide_ratio;
    command(display_clock_div);

    float osc_frequency_value = 1000.0f * (float) OSC_FREQUENCY_TABLE[osc_frequency_flags];

    int num_disp_clocks_per_row = 54;  // (2 + 2 + 50)
    int clock_ticks_per_frame = (clock_divide_ratio + 1) * (int) height_ * num_disp_clocks_per_row;
    frequency_ = osc_frequency_value / (float)(clock_ticks_per_frame);

    command(Command::SetMultiplexRatio);        // SSD1306_SETMULTIPLEX
    command(height_ - 1);                       // multiplex ratio: 1/64 or 1/32
    command(Command::SetDisplayOffset);         // SSD1306_SETDISPLAYOFFSET
    command(0x00);                              // 0 no display offset (default)
    command(static_cast<uint8_t>(Command::SetStartLine) + 0);         // SSD1306_SETSTARTLINE line #0

    if (type_ == PanelType::SSD1306_128x32) {
        command(Command::ChargePumpSetting);
        command(0x14);  // Charge pump on
    }

    command(Command::SetMemoryAddressingMode);
    command(static_cast<uint8_t>(MemoryMode::HorizontalMode));
    command(Command::SetSegmentRemapInverse);
    command(Command::SetComOutputScanDec);
    command(Command::SetComPinsHardwareConfiguration);
    if (type_ == PanelType::SSD1306_128x64) {
        command(0x12);
    } else if (type_ == PanelType::SSD1306_128x32) {
        command(0x02);
    }

    command(Command::SetContrastControl);
    if (type_ == PanelType::SSD1306_128x64) {
        command(0xcf);
    } else if (type_ == PanelType::SSD1306_128x32) {
        command(0x2f);
    }

    command(Command::SetPreChargePeriod);
    command(0xf1);
    command(Command::SetVComHDeselectLevel);
    command(0x30);

    if (type_ == PanelType::SSD1306_128x64) {
        command(Command::ChargePumpSetting);
        command(0x14);  // Charge pump on
    }

    command(Command::DeactivateScroll);
    command(Command::EntireDisplayOnResume);
    command(Command::SetNormalDisplay);

    clear();
    refresh(true);

    command(Command::SetDisplayOn);

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
    command(Command::SetDisplayOff);
    command(Command::ChargePumpSetting);
    command(0x10);  // Charge pump off

    if (buffer_) {
        free(buffer_);
        buffer_ = nullptr;
    }
}

uint8_t Device::width() {
    return width_;
}

uint8_t Device::height() {
    return height_;
}

void Device::clear() {
    if (buffer_size_ > 0) {
        memset(buffer_, 0, buffer_size_);
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

void Device::markRegion(const Rectangle& region) {
    markRegion(region.left, region.right, region.top, region.bottom);
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
        command(Command::SetInverseDisplay);
    else
        command(Command::SetNormalDisplay);
}

void Device::updateBuffer(uint8_t *data, uint16_t length) {
    memcpy(buffer_, data, (length < buffer_size_) ? length : buffer_size_);
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

        uint8_t buffer[] = {
            static_cast<uint8_t>(Command::SetColumnAddress), 0, (uint8_t) (width_ - 1),
            static_cast<uint8_t>(Command::SetPageAddress), 0, (uint8_t) (num_pages_ -1)
        };

        i2c->sendControlBuffer(buffer, sizeof(buffer));
        i2c->sendDataBuffer(buffer_, buffer_size_);

    } else {
        for (int page = 0; page < num_pages_; page++) {
            refreshPage(page, false);
        }
    }

    // reset dirty area
    clearRegions();
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

    uint8_t buffer[] = {
        static_cast<uint8_t>(Command::SetColumnAddress), col_start, col_end,
        static_cast<uint8_t>(Command::SetPageAddress), (uint8_t) page, (uint8_t) page
    };

    i2c->sendControlBuffer(buffer, sizeof(buffer));

    int page_size = width_;
    int page_offset = page * page_size;
    size_t bytes_to_send = col_end - col_start + 1;

    i2c->sendDataBuffer(buffer_ + page_offset + col_start, bytes_to_send);

    // clear dirty region
    page_info.dirty_left = 255;
    page_info.dirty_right = 0;
}

void Device::startHorizontalScrolling(int start_page, int end_page, bool right, int time_interval) {

    uint8_t buffer [] = {
        static_cast<uint8_t>(Command::DeactivateScroll),
        static_cast<uint8_t>(right ? Command::RightScroll : Command::LeftScroll),
        0x0, (uint8_t) start_page, (uint8_t) time_interval, (uint8_t) end_page, 0x0, 0xff,
        static_cast<uint8_t>(Command::ActivateScroll)
    };

    i2c->sendControlBuffer(buffer, sizeof(buffer));
}

void Device::startDiagonalScrolling(int start_page, int end_page, int start_row, int end_row, bool right, int time_interval, int vertical_offset) {

    int num_rows = end_row - start_row + 1;
    if (num_rows < 0) num_rows = 0;

    if (vertical_offset < 0) {
        vertical_offset = num_rows + vertical_offset;
    }

    auto c1 = Command::SetVerticalScrollArea;
    auto c2 = right ? Command::VerticalRightScroll : Command::VerticalLeftScroll;

    uint8_t buffer[] = {
        static_cast<uint8_t>(Command::DeactivateScroll),
        static_cast<uint8_t>(c1),
        (uint8_t) start_row, (uint8_t) num_rows,
        static_cast<uint8_t>(c2),
        0x0, (uint8_t) start_page, (uint8_t) time_interval, (uint8_t) end_page, (uint8_t) vertical_offset,
        static_cast<uint8_t>(Command::ActivateScroll)
    };

    i2c->sendControlBuffer(buffer, sizeof(buffer));
}

void Device::stopScrolling(void) {
    command(Command::DeactivateScroll);
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
