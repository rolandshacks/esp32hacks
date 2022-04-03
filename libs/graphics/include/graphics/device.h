//
// SSD1306 OLED Display Controller
//
#pragma once

#include <stddef.h>
#include <stdlib.h>

#include "base.h"
#include "sys/i2c.h"

namespace graphics {

//! @brief Panel type
typedef enum {
    SSD1306_128x64 = 1,  //!< 128x32 panel
    SSD1306_128x32 = 2   //!< 128x64 panel
} panel_type_t;

class Device {
    public:
        /**
         * @brief   Constructor
         */
        Device();

        /**
         * @brief   Constructor
         * @param   scl_pin  SCL Pin
         * @param   sda_pin  SDA Pin
         * @param   type     Panel type
         */
        Device(gpio_num_t scl, gpio_num_t sda, panel_type_t type);

        /**
         * @brief   Constructor
         * @param   scl_pin  SCL Pin
         * @param   sda_pin  SDA Pin
         * @param   type     Panel type
         * @param   address  I2C Address(usually 0x78)
         */
        Device(gpio_num_t scl, gpio_num_t sda, panel_type_t type, uint8_t address);
        /**
         * @brief   Initialize OLED panel
         * @return  true if successful
         * @remark  Possible reasons for failure include non-configured panel type,
         * out of memory or I2C not responding
         */
        bool init();

        /**
         * @brief   De-initialize OLED panel, turn off power and free memory
         * @return  true if successful
         * @remark  Possible reasons for failure include non-configured panel type,
         * out of memory or I2C not responding
         */
        void term();

        /**
         * @brief   Return OLED panel height
         * @return  Panel height, or return 0 if failed (panel not initialized)
         */
        uint8_t width();

        /**
         * @brief   Return OLED panel height
         * @return  Panel height, or return 0 if failed (panel not initialized)
         */
        uint8_t height();

        /**
         * @brief   Clear display buffer (fill with black)
         */
        void clear();

        /**
         * @brief   Refresh display (send display buffer to the panel)
         *          The program automatically tracks "dirty" regions to minimize
         * refresh area.
         */
        void refresh();

        /**
         * @brief   Refresh display (send display buffer to the panel)
         * @param   force   The program automatically tracks "dirty" regions to
         * minimize refresh area. Set #force to true to ignore the dirty region and
         * refresh the whole screen.
         */
        void refresh(bool force);

        /**
         * @brief   Refresh display page (send display page buffer to the panel)
         * @param   page    page number
         * @param   force   force update without dirty region check
         */
        void refreshPage(int page, bool force = true);

        /**
         * @brief   Set normal or inverted display
         * @param   invert      Invert display?
         */
        void invertDisplay(bool invert);


        /**
         * @brief   Direct update display buffer
         * @param   data        Data to fill display buffer, no length check is performed!
        * @param   length      Length of data
        */
        void updateBuffer(uint8_t* data, uint16_t length);

        /*!
            @brief  Activate continuous horizontal scrolling.
            @param  start_page
                    Start page.
            @param  end_page
                    End page.
            @param  right
                    Scroll to right if true, otherwise scroll to left.
            @param  time_interval
                    Interval bit mask. (000b – 5 frames, 001b – 64 frames, 010b –
                    128 frames, 011b – 256 frames, 100b – 3 frames, 101b – 4 frames, 110b –
                    25 frame, 111b – 2 frames)
            @return None (void).
        */
        void startHorizontalScrolling(int start_page, int end_page, bool right, int time_interval);

        /*!
            @brief  Activate continuous diagonal scrolling. Plain vertical scroll is not possible.
            @param  start_page
                    Start page for scrolling (both horizontal and vertical).
            @param  end_page
                    End page for scrolling (both horizontal and vertical).
            @param  start_row
                    Start row for vertical scrolling.
            @param  end_row
                    End row for vertical scrolling.
            @param  right
                    Scroll to right if true, otherwise scroll to left.
            @param  time_interval
                    Interval bit mask. (000b – 5 frames, 001b – 64 frames, 010b –
                    128 frames, 011b – 256 frames, 100b – 3 frames, 101b – 4 frames, 110b –
                    25 frame, 111b – 2 frames)
            @param  vertical_offset
                    Vertical offset per scrolling step. If zero, plain horizontal scrolling is performed.
            @return None (void).
        */
        void startDiagonalScrolling(int start_page, int end_page, int start_row, int end_row, bool right, int time_interval, int vertical_offset);

        /*!
            @brief  Cease a previously-begun scrolling action.
            @return None (void).
        */
        void stopScrolling();

        /*!
            @brief  Enable partial update.
            @param  enable
                    Enable or disable partial updates to avoid display data transfer
            @return None (void).
        */
        void enablePartialUpdates(bool enable = true);

        /*!
            @brief  Get partial update flag.
            @return Enable status of partial updates
        */
        bool isPartialUpdatesEnabled() const;

        /*!
            @brief  Lock display memory page.
            @param  page
                    Memory page number
            @param  lock
                    Lock status on or off
            @return None (void).
        */
        void lockPage(int page, bool lock=true);

        /*!
            @brief  Get display frequency
            @return Display frequency
        */
        float frequency() const;

        /*!
            @brief  Set vertical offset
            @param  ofs
                    Vertical offset (0..height-1)
            @return None (void).
        */
        void setVerticalOffset(int ofs);

    private:
        /*!
            @brief  Send command to SSD1306.
            @param  c
                    Command byte
            @return None (void).
        */
        void command(uint8_t c);

        /*!
            @brief  Send commands to SSD1306.
            @param  c0
                    Command byte
            @param  c1
                    Command byte, if < 0 it will not be sent.
            @param  c2
                    Command byte, if < 0 it will not be sent.
            @param  c3
                    Command byte, if < 0 it will not be sent.
            @param  c4
                    Command byte, if < 0 it will not be sent.
            @param  c5
                    Command byte, if < 0 it will not be sent.
            @param  c6
                    Command byte, if < 0 it will not be sent.
            @param  c7
                    Command byte, if < 0 it will not be sent.
            @param  c8
                    Command byte, if < 0 it will not be sent.
            @return None (void).
        */
        void command(int c0, int c1, int c2 = -1, int c3 = -1, int c4 = -1, int c5 = -1, int c6 = -1, int c7 = -1, int c8 = -1);

        /*!
            @brief  Issue list of commands to SSD1306, same rules as above re:
           transactions.
            @param  c
                    Byte sequence
            @param  n
                    Number of bytes
        */
        void command_list(const uint8_t* c, uint8_t n);

        /*!
            @brief  Send data to SSD1306.
            @param  d
                    Data byte
            @return None (void).
        */
        void data(uint8_t d);

        /*!
            @brief  Set or increase page dirty region
            @param  x_start
                    Start of dirty region
            @param  x_end
                    End of dirty region
            @param  page
                    Page number
            @return None (void).
        */
        void setPageRegion(int x_start, int x_end, int page);

    public:
        /*!
            @brief  Mark dirty region
            @param  x
                    X-position
            @param  y
                    Y-position
            @return None (void).
        */
        void markRegion(int x, int y);

        /*!
            @brief  Mark dirty region
            @param  x_start
                    Horizontal start position
            @param  x_end
                    Horizontal end position
            @param  y
                    Y-position
            @return None (void).
        */
        void markRegion(int x_start, int x_end, int y);

        /*!
            @brief  Mark dirty region
            @param  x_start
                    Horizontal start position
            @param  x_end
                    Horizontal end position
            @param  y_start
                    Vertical start position
            @param  y_end
                    Vertical end position
            @return None (void).
        */
        void markRegion(int x_start, int x_end, int y_start, int y_end);

        /*!
            @brief  Clear dirty regions
            @return None (void).
        */
        void clearRegions();

    public:
        /*!
            @brief  Get display buffer.
            @return Pointer to display memory buffer
        */
        uint8_t* buffer();

        /*!
            @brief  Get display buffer.
            @return Pointer to display memory buffer
        */
        const uint8_t* buffer() const;

        /*!
            @brief  Get display buffer size.
            @return Display buffer size in bytes
        */
        uint16_t bufferSize() const;

    private:
        sys::I2C* i2c;                  // I2C connection
        panel_type_t type_;             // panel type
        uint8_t address_;               // I2C address
        uint8_t* buffer_;               // display buffer
        uint16_t buffer_size_;          // display buffer size
        uint8_t width_;                 // panel width (128)
        uint8_t height_;                // panel height (32 or 64)
        uint8_t num_pages_;             // number of pages (4 or 8)
        bool partial_updates_enabled_;  // enable dirty regions handling
        float frequency_;               // display frequency
        Page page_info_[8];             // page_info

    public:
        Device(const Device&) = delete;
        Device(const Device&&) = delete;
        Device& operator=(const Device&) = delete;
        Device& operator=(const Device&&) = delete;
        ~Device() = default;
};

}  // namespace graphics
