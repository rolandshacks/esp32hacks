#pragma once

#include <cstdint>
#include <vector>

const uint8_t DISPLAY_TYPE_SSD1306_128x64 = 0x12;
const uint8_t DISPLAY_TYPE_SSD1306_128x32 = 0x02;

class EmuSSD1306 {
   public:
    EmuSSD1306();

   public:
    void reset();

   public:
    void onCommand(const uint8_t* data, size_t data_size);
    void onData(const uint8_t* data, size_t data_size);

   private:
    void requestCommandData(size_t sz = 1);
    void pushCommandData(uint8_t data);
    void clearCommandData();

   private:
    void alloc();
    void store(const uint8_t* data, size_t data_size);
    void trap();
    void scroll();

   public:
    bool update();
    const uint8_t* getBuffer() const;
    size_t getBufferSize() const;
    bool getPixel(int x, int y) const;
    uint32_t getCycleTimeMs() const;

   private:
    uint8_t last_command_{0};
    std::vector<uint8_t> command_buffer_;
    size_t command_buffer_required_{0};
    size_t command_buffer_usage_{0};

   private:
    bool display_on_{false};
    uint8_t display_type_{DISPLAY_TYPE_SSD1306_128x64};
    uint8_t display_width_{128};
    uint8_t display_height_{64};
    size_t display_page_size_{0};
    size_t display_memory_size_{0};

    uint32_t display_cycle_time_ms_{0};
    uint8_t display_osc_frequency_{0};
    uint8_t display_clock_divide_ratio_{0};
    uint8_t display_contrast_{0};
    uint8_t display_start_line_{0};
    uint8_t display_offset_{0};
    uint8_t display_multiplex_{0};
    uint8_t display_column_start_{0};
    uint8_t display_column_end_{0};
    uint8_t display_column_addr_{0};
    uint8_t display_row_addr_{0};
    uint8_t display_page_start_{0};
    uint8_t display_page_end_{0};
    uint8_t display_page_addr_{0};
    uint8_t display_page_count_{8};
    uint8_t display_page_addr_mode_{0};

    bool display_scroll_enabled_{false};
    uint8_t display_scroll_mode_{0};
    uint8_t display_scroll_time_intervall_{0};
    uint8_t display_scroll_page_start_{0};
    uint8_t display_scroll_page_end_{0};
    uint8_t display_scroll_vertical_inc_{0};
    uint8_t display_scroll_vertical_start_{0};
    uint8_t display_scroll_vertical_num_{64};

    uint32_t display_scroll_frame_counter_{0};
    uint32_t display_scroll_vertical_offset_{0};

   private:
    static const uint8_t SCROLL_FRAME_INTERVAL[];
    static const uint16_t OSC_FREQUENCY_TABLE[];

   private:
    bool display_fault_{false};

    std::vector<uint8_t> buffer_;
    uint8_t buffer_addr_{0};

    uint64_t time_offset_{0};
    uint64_t time_next_update_{0};

};
