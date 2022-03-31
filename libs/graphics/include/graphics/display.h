//
// SSD1306 OLED Display Controller
//
#pragma once

#include <stddef.h>
#include <stdlib.h>

#include <string>

#include "base.h"
#include "fonts.h"
#include "bitmaps.h"
#include "sys/i2c.h"

namespace graphics {

//! @brief Drawing color
typedef enum {
    TRANSPARENT = -1,  //!< Transparent (not drawing)
    BLACK = 0,         //!< Black (pixel off)
    WHITE = 1,         //!< White (or blue, yellow, pixel on)
    INVERT = 2,        //!< Invert pixel (XOR)
} color_t;

//! @brief Panel type
typedef enum {
    SSD1306_128x64 = 1,  //!< 128x32 panel
    SSD1306_128x32 = 2   //!< 128x64 panel
} panel_type_t;

class Display {
    public:
        /**
         * @brief   Constructor
         */
        Display();

        /**
         * @brief   Constructor
         * @param   scl_pin  SCL Pin
         * @param   sda_pin  SDA Pin
         * @param   type     Panel type
         */
        Display(gpio_num_t scl, gpio_num_t sda, panel_type_t type);

        /**
         * @brief   Constructor
         * @param   scl_pin  SCL Pin
         * @param   sda_pin  SDA Pin
         * @param   type     Panel type
         * @param   address  I2C Address(usually 0x78)
         */
        Display(gpio_num_t scl, gpio_num_t sda, panel_type_t type, uint8_t address);
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
        uint8_t get_width();

        /**
         * @brief   Return OLED panel height
         * @return  Panel height, or return 0 if failed (panel not initialized)
         */
        uint8_t get_height();

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
        void refresh_page(int page, bool force = true);

        /**
         * @brief   Draw one pixel
         * @param   x       X coordinate
         * @param   y       Y coordinate
         * @param   color   Color of the pixel
         */
        void draw_pixel(int8_t x, int8_t y, color_t color);

        /**
         * @brief   Draw horizontal line
         * @param   x       X coordinate or starting (left) point
         * @param   y       Y coordinate or starting (left) point
         * @param   w       Line width
         * @param   color   Color of the line
         */
        void draw_hline(int8_t x, int8_t y, uint8_t w, color_t color);

        /**
         * @brief   Draw vertical line
         * @param   x       X coordinate or starting (top) point
         * @param   y       Y coordinate or starting (top) point
         * @param   h       Line height
         * @param   color   Color of the line
         */
        void draw_vline(int8_t x, int8_t y, uint8_t h, color_t color);

        /**
         * @brief   Draw a rectangle
         * @param   x       X coordinate or starting (top left) point
         * @param   y       Y coordinate or starting (top left) point
         * @param   w       Rectangle width
         * @param   h       Rectangle height
         * @param   color   Color of the rectangle border
         */
        void draw_rectangle(int8_t x, int8_t y, uint8_t w, uint8_t h, color_t color);

        /**
         * @brief   Draw a filled rectangle
         * @param   x       X coordinate or starting (top left) point
         * @param   y       Y coordinate or starting (top left) point
         * @param   w       Rectangle width
         * @param   h       Rectangle height
         * @param   color   Color of the rectangle
         */
        void fill_rectangle(int8_t x, int8_t y, uint8_t w, uint8_t h, color_t color);

        /**
         * @brief   Draw a filled circle
         * @param   x0      X coordinate or center
         * @param   y0      Y coordinate or center
         * @param   r       Radius
         * @param   color   Color of the circle
         */
        void draw_circle(int8_t x0, int8_t y0, uint8_t r, color_t color);

        /**
         * @brief   Draw a filled circle
         * @param   x0      X coordinate or center
         * @param   y0      Y coordinate or center
         * @param   r       Radius
         * @param   color   Color of the circle
         */
        void fill_circle(int8_t x0, int8_t y0, uint8_t r, color_t color);

        /**
         * @brief   Select font for drawing
         * @param   idx     Font index, see fonts.c
         */
        void select_font(uint8_t idx);

        /**
         * @brief   Draw monochrome bitmap
         * @param   bitmap      Bitmap to be drawn
         * @param   x           X position (top-left corner)
         * @param   y           Y position (top-left corner)
         * @param   width       Width of bitmap
         * @param   height      Height of bitmap
         * @param   foreground  Pixel color
         * @param   background  Background color
         */
        void draw_bitmap(const uint8_t* bitmap, int x, int y, int width, int height, color_t foreground=WHITE, color_t background=BLACK);

        /**
         * @brief   Draw monochrome bitmap
         * @param   bitmap      Bitmap to be drawn
         * @param   x           X position (top-left corner)
         * @param   y           Y position (top-left corner)
         * @param   foreground  Pixel color
         * @param   background  Background color
         */
        void draw_bitmap(const bitmap_t* bitmap, int x, int y, bool enable_alpha=true, color_t foreground=WHITE, color_t background=BLACK);

        /**
         * @brief   Draw monochrome bitmap
         * @param   bitmap      Bitmap to be drawn
         * @param   mask_bitmap Bitmap mask to be used
         * @param   x           X position (top-left corner)
         * @param   y           Y position (top-left corner)
         * @param   width       Width of bitmap
         * @param   height      Height of bitmap
         * @param   foreground  Pixel color
         * @param   background  Background color
         */
        void draw_masked_bitmap(const uint8_t* bitmap, const uint8_t* mask_bitmap, int x, int y, int width, int height,
                                color_t foreground=WHITE, color_t background=BLACK);

        /**
         * @brief   Draw one character using currently selected font
         * @param   x           X position of character (top-left corner)
         * @param   y           Y position of character (top-left corner)
         * @param   c           The character to draw
         * @param   foreground  Character color
         * @param   background  Background color
         * @return  Width of the character
         */
        uint8_t draw_char(uint8_t x, uint8_t y, int c, color_t foreground=WHITE, color_t background=BLACK);

        /**
         * @brief   Draw string using currently selected font
         * @param   x           X position of string (top-left corner)
         * @param   y           Y position of string (top-left corner)
         * @param   str         The string to draw
         * @param   foreground  Character color
         * @param   background  Background color
         * @return  Width of the string (out-of-display pixels also included)
         */
        uint8_t draw_string(uint8_t x, uint8_t y, const std::string& str, color_t foreground=WHITE, color_t background=BLACK);

        /**
         * @brief   Draw string using currently selected font
         * @param   x           X position of string (top-left corner)
         * @param   y           Y position of string (top-left corner)
         * @param   str         The string to draw
         * @param   foreground  Character color
         * @param   background  Background color
         * @return  Width of the string (out-of-display pixels also included)
         */
        uint8_t draw_string(uint8_t x, uint8_t y, const char* str, color_t foreground=WHITE, color_t background=BLACK);

        /**
         * @brief   Measure width of string with current selected font
         * @param   str         String to measure
         * @return  Width of the string
         */
        uint8_t measure_string(const std::string& str);

        /**
         * @brief   Measure width of string with current selected font
         * @param   str         String to measure
         * @return  Width of the string
         */
        uint8_t measure_string(const char* str);

        /**
         * @brief   Get the height of current selected font
         * @return  Height of the font (in pixels) or 0 if none font selected
         */
        uint8_t get_font_height();

        /**
         * @brief   Get the "C" value (space between adjacent characters)
         * @return  "C" value
         */
        uint8_t get_font_c();

        /**
         * @brief   Set normal or inverted display
         * @param   invert      Invert display?
         */
        void invert_display(bool invert); /**

             * @brief   Direct update display buffer
             * @param   data        Data to fill display buffer, no length check is
             performed!
             * @param   length      Length of data
             */
        void update_buffer(uint8_t* data, uint16_t length);

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
        void start_scroll_horizontal(int start_page, int end_page, bool right, int time_interval);

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
        void start_scroll(int start_page, int end_page, int start_row, int end_row, bool right, int time_interval, int vertical_offset);

        /*!
            @brief  Cease a previously-begun scrolling action.
            @return None (void).
        */
        void stop_scroll();

        void enable_partial_updates(bool enable = true);
        bool is_partial_updates_enabled() const;
        void set_page_lock(int page, bool lock = true);
        float get_frequency() const;
        void set_vertical_offset(int ofs);

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

        void draw_pixel_raw(int8_t x, int8_t y, color_t color);
        void mark_region(int x, int y);
        void mark_region(int x_start, int x_end, int y);
        void mark_region(int x_start, int x_end, int y_start, int y_end);
        void set_page_region(int x_start, int x_end, int page);
        void clear_regions();

    private:
        sys::I2C* i2c;                  // I2C connection
        panel_type_t type_;             // panel type
        uint8_t address_;               // I2C address
        uint8_t* buffer_;               // display buffer
        uint16_t buffer_size_;          // display buffer size
        uint8_t width_;                 // panel width (128)
        uint8_t height_;                // panel height (32 or 64)
        uint8_t num_pages_;             // number of pages (4 or 8)
        const font_info_t* font_;       // current font
        bool partial_updates_enabled_;  // enable dirty regions handling
        float frequency_;               // display frequency
        Page page_info_[8];             // page_info

    public:
        Display(const Display&) = delete;
        Display(const Display&&) = delete;
        Display& operator=(const Display&) = delete;
        Display& operator=(const Display&&) = delete;
        ~Display() = default;
};

}  // namespace graphics
