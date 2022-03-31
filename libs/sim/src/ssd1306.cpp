//
// Sim
//

#include "sim/ssd1306.h"

#include <cstdio>
#include <chrono>

const uint8_t EmuSSD1306::SCROLL_FRAME_INTERVAL[] = {6, 32, 64, 128,
                                                     3, 4,  5,  2};

/// Oscillator frequency table for 0xD5 command
const uint16_t EmuSSD1306::OSC_FREQUENCY_TABLE[] = {270, 279, 289, 298, 314, 326, 337, 352,
                                                    372, 391, 409, 431, 451, 477, 506, 536};

EmuSSD1306::EmuSSD1306() { reset(); }

void EmuSSD1306::reset() {
    display_on_ = false;

    display_type_ = DISPLAY_TYPE_SSD1306_128x64;
    display_width_ = 128;
    display_height_ = 64;
    display_memory_size_ = (int)display_height_ * display_width_ / 8;

    display_osc_frequency_ = 0x8;
    display_clock_divide_ratio_ = 0x0;
    display_cycle_time_ms_ = 57;

    display_start_line_ = 0x0;

    display_column_start_ = 0x0;
    display_column_end_ = 0x0;
    display_column_addr_ = 0x0;
    display_row_addr_ = 0x0;

    display_page_start_ = 0x0;
    display_page_end_ = 0x7;
    display_page_addr_ = 0x0;
    display_page_count_ = 8;
    display_page_size_ = display_memory_size_ / display_page_count_;

    display_contrast_ = 0x7f;

    display_scroll_enabled_ = false;
    display_scroll_mode_ = 0x0;
    display_scroll_time_intervall_ = 0x0;
    display_scroll_page_start_ = 0x0;
    display_scroll_page_end_ = 0x0;
    display_scroll_vertical_inc_ = 0x0;
    display_scroll_vertical_start_ = 0x0;
    display_scroll_vertical_num_ = display_height_;

    display_scroll_frame_counter_ = 0;
    display_scroll_vertical_offset_ = 0;

    alloc();
}

void EmuSSD1306::alloc() {
    size_t buffer_size = display_height_ * display_width_ / 8;
    buffer_.resize(buffer_size, 0x0);
    buffer_addr_ = 0x0;

    command_buffer_.resize(1024, 0x0);
    command_buffer_usage_ = 0x0;
    command_buffer_required_ = 0x0;
}

void EmuSSD1306::pushCommandData(uint8_t data) {
    if (command_buffer_usage_ >= command_buffer_.size()) {
        return;
    }

    command_buffer_[command_buffer_usage_++] = data;
}

void EmuSSD1306::requestCommandData(size_t sz) {
    command_buffer_usage_ = 0x0;
    command_buffer_required_ = sz;
}

void EmuSSD1306::clearCommandData() { requestCommandData(0); }

#define dump_opcode(OPCODE) /* */
//#define dump_opcode(OPCODE) \
//printf("OPCODE: %s\n", #OPCODE)

void EmuSSD1306::onCommand(const uint8_t* data, size_t data_size) {

    if (nullptr == data) {
        clearCommandData();
        last_command_ = 0x0;
        return;
    }

    size_t ofs = 0;
    size_t avail = data_size;

    while (avail > 0) {

        if (0 == command_buffer_required_) {

            // get next command
            uint8_t command = data[ofs];
            ofs++; avail--;

            if (command >= 0x40 && command <= 0x7f) {
                // Set display RAM display start line register from 0-63
                display_start_line_ = (command & 0x3f);
                last_command_ = 0x0;
            } else {
                clearCommandData();

                switch (command) {
                    case 0x21:  // SSD1306_COLUMNADDR
                        requestCommandData(2);
                        break;
                    case 0x22:  // SSD1306_PAGEADDR
                        requestCommandData(2);
                        break;
                    case 0x2f:  // SSD1306_ACTIVATE_SCROLL
                        dump_opcode(SSD1306_ACTIVATE_SCROLL);
                        display_scroll_frame_counter_ = 0;
                        display_scroll_enabled_ = true;
                        break;
                    case 0x2e:  // SSD1306_DEACTIVATE_SCROLL
                        dump_opcode(SSD1306_DEACTIVATE_SCROLL);
                        display_scroll_frame_counter_ = 0;
                        display_scroll_enabled_ = false;
                        break;
                    case 0x26:  // SSD1306_RIGHT_HORIZONTAL_SCROLL
                        requestCommandData(6);
                        break;
                    case 0x27:  // SSD1306_LEFT_HORIZONTAL_SCROLL
                        requestCommandData(6);
                        break;
                    case 0x29:  // SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL
                        requestCommandData(5);
                        break;
                    case 0x2a:  // SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL
                        requestCommandData(5);
                        break;
                    case 0xa3:  // SSD1306_SET_VERTICAL_SCROLL_AREA
                        requestCommandData(2);
                        break;
                    case 0xa4:  // SSD1306_DISPLAYALLON_RESUME
                        break;
                    case 0xa6:  // SSD1306_NORMALDISPLAY
                        break;
                    case 0xae:  // SSD1306_DISPLAYOFF
                        dump_opcode(SSD1306_DISPLAYOFF);
                        display_on_ = false;
                        break;
                    case 0xaf:  // SSD1306_DISPLAYON
                        dump_opcode(SSD1306_DISPLAYON);
                        display_on_ = true;
                        break;
                    case 0xd5:  // SSD1306_SETDISPLAYCLOCKDIV
                        requestCommandData();
                        break;
                    case 0xa8:  // SSD1306_SETMULTIPLEX
                        requestCommandData();
                        break;
                    case 0xd3:  // SSD1306_SETDISPLAYOFFSET
                        requestCommandData();
                        break;
                    case 0x81:  // SSD1306_SETCONTRAST
                        requestCommandData();
                        break;
                    case 0x8d:  // SSD1306_CHARGEPUMP
                        requestCommandData();
                        break;
                    case 0xd9:  // SSD1306_SETPRECHARGE
                        requestCommandData();
                        break;
                    case 0xdb:  // SSD1306_SETVCOMDETECT
                        requestCommandData();
                        break;
                    case 0x20:  // SSD1306_MEMORYMODE
                        requestCommandData();
                        break;
                    case 0xa1:  // SSD1306_SEGREMAP
                        requestCommandData();
                        break;
                    case 0xda:  // SSD1306_SETCOMPINS
                        requestCommandData();
                        break;
                    default:
                        printf("unhandled opcode 0x%02x\n", (int)command);
                        break;
                }

                last_command_ = command;
            }

        } else {

            // process command parameter data

            while (avail > 0) {
                if (command_buffer_usage_ >= command_buffer_required_) {
                    break;
                }
                pushCommandData(data[ofs]);
                ofs++; avail--;
            }

            if (command_buffer_usage_ >= command_buffer_required_) {
                switch (last_command_) {
                    case 0xa8:  // SSD1306_SETMULTIPLEX
                        display_multiplex_ = command_buffer_[0];
                        alloc();
                        dump_opcode(SSD1306_SETMULTIPLEX);
                        break;
                    case 0xd3:  // SSD1306_SETDISPLAYOFFSET
                        display_offset_ = command_buffer_[0];
                        dump_opcode(SSD1306_SETDISPLAYOFFSET);
                        break;
                    case 0x20:  // SSD1306_MEMORYMODE
                        display_page_addr_mode_ = command_buffer_[0];
                        break;
                    case 0x21:  // SSD1306_COLUMNADDR
                        display_column_start_ = command_buffer_[0];
                        display_column_end_ = command_buffer_[1];
                        display_column_addr_ = display_column_start_;
                        display_row_addr_ = 0;
                        dump_opcode(SSD1306_COLUMNADDR);
                        break;
                    case 0x22:  // SSD1306_PAGEADDR
                        display_page_start_ = command_buffer_[0];
                        display_page_end_ = command_buffer_[1];
                        display_page_addr_ = display_page_start_;
                        dump_opcode(SSD1306_PAGEADDR);
                        break;
                    case 0x26:  // SSD1306_RIGHT_HORIZONTAL_SCROLL
                        display_scroll_mode_ = last_command_;
                        display_scroll_page_start_ = command_buffer_[1] & 0b111;
                        display_scroll_time_intervall_ = command_buffer_[2] & 0b111;
                        display_scroll_page_end_ = command_buffer_[3] & 0b111;
                        display_scroll_vertical_inc_ = 0;
                        display_scroll_vertical_offset_ = 0;
                        break;
                    case 0x27:  // SSD1306_LEFT_HORIZONTAL_SCROLL
                        display_scroll_mode_ = last_command_;
                        display_scroll_page_start_ = command_buffer_[1] & 0b111;
                        display_scroll_time_intervall_ = command_buffer_[2] & 0b111;
                        display_scroll_page_end_ = command_buffer_[3] & 0b111;
                        display_scroll_vertical_inc_ = 0;
                        display_scroll_vertical_offset_ = 0;
                        break;
                    case 0x29:  // SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL
                        display_scroll_mode_ = last_command_;
                        display_scroll_page_start_ = command_buffer_[1] & 0b111;
                        display_scroll_time_intervall_ = command_buffer_[2] & 0b111;
                        display_scroll_page_end_ = command_buffer_[3] & 0b111;
                        display_scroll_vertical_inc_ = command_buffer_[4] & 0x3f;
                        display_scroll_vertical_offset_ = 0;
                        break;
                    case 0x2a:  // SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL
                        display_scroll_mode_ = last_command_;
                        display_scroll_page_start_ = command_buffer_[1] & 0b111;
                        display_scroll_time_intervall_ = command_buffer_[2] & 0b111;
                        display_scroll_page_end_ = command_buffer_[3] & 0b111;
                        display_scroll_vertical_inc_ = command_buffer_[4] & 0x3f;
                        display_scroll_vertical_offset_ = 0;
                        break;
                    case 0xa3:  // SSD1306_SET_VERTICAL_SCROLL_AREA
                        display_scroll_vertical_start_ = command_buffer_[0];
                        display_scroll_vertical_num_ = command_buffer_[1];
                        break;
                    case 0x81:  // SSD1306_SETCONTRAST
                        display_contrast_ = command_buffer_[0];
                    case 0x8d:  // SSD1306_CHARGEPUMP
                        break;
                    case 0xd5:  // SSD1306_SETDISPLAYCLOCKDIV
                    {
                        display_osc_frequency_ = (command_buffer_[0] >> 4);
                        display_clock_divide_ratio_ = (command_buffer_[0] & 0x0f);
                        int num_disp_clocks_per_row = 54;  // (2 + 2 + 50)
                        float osc_frequency_value = 1000.0f * (float) OSC_FREQUENCY_TABLE[display_osc_frequency_];
                        int clock_ticks_per_frame = (display_clock_divide_ratio_ + 1) * display_height_ * num_disp_clocks_per_row;
                        float frequency = osc_frequency_value / (float)(clock_ticks_per_frame);
                        float cycle_time = (frequency != 0.0f) ? 1.0f / frequency : 0.0f;
                        display_cycle_time_ms_ = (uint32_t) (1000.0 * cycle_time);
                        break;
                    }
                    default:
                        // ignore
                        printf("ignore opcode 0x%02x\n", (int)last_command_);
                        break;
                }

                clearCommandData();
            }

            // return;  // skip further processing
        }
    }
}

void EmuSSD1306::onData(const uint8_t* data, size_t data_size) {
    if (0x21 == last_command_ || 0x22 == last_command_) {
        // printf("I2C: store data (%d data bytes)\n", (int) data_size);
        store(data, data_size);
    } else {
        printf("I2C: handle data (%d data bytes)\n", (int)data_size);
        store(data, data_size);
    }
}

void EmuSSD1306::trap() { display_fault_ = true; }

void EmuSSD1306::store(const uint8_t* data, size_t data_size) {
    for (auto i = 0; i < data_size; i++) {
        size_t ofs =
            display_page_addr_ * display_page_size_ + display_column_addr_;
        buffer_[ofs] = *(data++);

        if (0x0 == display_page_addr_mode_) {  // Horizontal addressing mode
            display_column_addr_++;
            if (display_column_addr_ > display_column_end_) {
                display_column_addr_ = display_column_start_;
                display_page_addr_++;
                if (display_page_addr_ > display_page_end_) {
                    display_page_addr_ = display_page_start_;
                }
            }
        } else if (0x1 ==
                   display_page_addr_mode_) {  // Vertical addressing mode
            display_page_addr_++;
            if (display_page_addr_ > display_page_end_) {
                display_page_addr_ = display_page_start_;
                display_column_addr_++;
                if (display_column_addr_ > display_column_end_) {
                    display_column_addr_ = display_column_start_;
                }
            }
        } else if (0x2 == display_page_addr_mode_) {  // Page addressing mode
            display_column_addr_++;
            if (display_column_addr_ > display_column_end_) {
                display_column_addr_ = display_column_start_;
                // no change of page address
            }
        }
    }
}

const uint8_t* EmuSSD1306::getBuffer() const { return buffer_.data(); }

size_t EmuSSD1306::getBufferSize() const { return buffer_.size(); }

bool EmuSSD1306::getPixel(int x, int y) const {
    if (x < 0 || x >= display_width_ || y < 0 || y >= display_height_) {
        return 0x0;
    }

    if (display_scroll_enabled_ && (display_scroll_mode_ == 0x29 || display_scroll_mode_ == 0x2a)) {
        if (y >= display_scroll_vertical_start_ && y < display_scroll_vertical_start_ + display_scroll_vertical_num_ && display_scroll_vertical_num_ > 0) {
            // y = display_scroll_vertical_start_ + ((y - display_scroll_vertical_start_ + display_scroll_vertical_offset_) % display_scroll_vertical_num_);
            y = display_scroll_vertical_start_ + ((y - display_scroll_vertical_start_ + display_scroll_vertical_offset_) % display_scroll_vertical_num_);
        }
    }

    size_t ofs = x + (y / 8) * display_width_;
    uint8_t segment = buffer_.at(ofs);
    bool pixel = (0x0 != (segment & (1 << (y & 7))));

    return pixel;
}

bool EmuSSD1306::update() {

    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (0 == time_offset_) {
        time_offset_ = now;
        now = 0;
    } else {
        now -= time_offset_;
    }

    uint32_t cycle_time_ms = getCycleTimeMs();

    if (now < time_next_update_) {
        return false;
    }

    time_next_update_ += cycle_time_ms;
    if (time_next_update_ < now) time_next_update_ = now; // correct delays

    if (display_scroll_enabled_) {
        scroll();
    }

    return true;
}

uint32_t EmuSSD1306::getCycleTimeMs() const {
    return display_cycle_time_ms_;
}

void EmuSSD1306::scroll() {

    uint32_t scroll_interval = SCROLL_FRAME_INTERVAL[display_scroll_time_intervall_];
    display_scroll_frame_counter_++;
    if (display_scroll_frame_counter_ <= scroll_interval) {
        return;
    }

    display_scroll_frame_counter_ = 0;

    if (0 != display_scroll_vertical_num_ && (0x29 == display_scroll_mode_ || 0x2a == display_scroll_mode_)) {
        display_scroll_vertical_offset_ = (display_scroll_vertical_offset_ + display_scroll_vertical_inc_) % display_scroll_vertical_num_;
    }

    for (auto page = display_scroll_page_start_; page <= display_scroll_page_end_; page++) {
        if (0x26 == display_scroll_mode_ || 0x29 == display_scroll_mode_) {
            uint8_t swap_buffer = buffer_[page * display_page_size_ + 127];
            for (int col = (int) display_page_size_ - 1; col > 0x0; col--) {
                int ofs = page * display_page_size_ + col;
                buffer_[ofs] = buffer_[ofs - 1];
            }
            buffer_[page * display_page_size_] = swap_buffer;
        } else if (0x27 == display_scroll_mode_ || 0x2a == display_scroll_mode_) {
            uint8_t swap_buffer = buffer_[page * display_page_size_];
            for (int col = 0x0; col < (int) display_page_size_ - 1; col++) {
                int ofs = page * display_page_size_ + col;
                buffer_[ofs] = buffer_[ofs + 1];
            }
            buffer_[page * display_page_size_ + display_page_size_ - 1] = swap_buffer;
        }
    }
}
