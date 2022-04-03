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
#include "device.h"

namespace graphics {

class Display {
    public:
        /**
         * @brief   Constructor
         */
        Display();

        /**
         * @brief   Initialize OLED panel
         * @return  true if successful
         * @remark  Possible reasons for failure include non-configured panel type,
         *          out of memory or I2C not responding
         */
        bool init();

        /**
         * @brief   Return OLED panel height
         * @return  Panel height, or return 0 if failed (panel not initialized)
         */
        int width();

        /**
         * @brief   Return OLED panel height
         * @return  Panel height, or return 0 if failed (panel not initialized)
         */
        int height();

        /**
         * @brief   Clear display buffer (fill with black)
         */
        void clear();

        /**
         * @brief   Set foreground color
         * @return  Previous foreground color
         * @param   foreground Color
         */
        color_t setForeground(color_t foreground);

        /**
         * @brief   Get current foreground color
         * @return   Current foreground color
         */
        color_t getForeground() const;

        /**
         * @brief   Set background color
         * @return  Previous background color
         * @param   background Color
         */
        color_t setBackground(color_t background);

        /**
         * @brief   Get current background color
         * @return  Current background color
         */
        color_t getBackground() const;

        /**
         * @brief   Draw one pixel
         * @param   x       X coordinate
         * @param   y       Y coordinate
         */
        void drawPixel(int x, int y);

        /**
         * @brief   Draw one pixel
         * @param   x       X coordinate
         * @param   y       Y coordinate
         * @param   color   Pixel color
         */
        void drawPixel(int x, int y, color_t color);

        /**
         * @brief   Draw line
         * @param   x1      X start coordinate
         * @param   y1      Y start coordinate
         * @param   x2      X end coordinate
         * @param   y2      Y end coordinate
         */
        void drawLine(int x1, int y1, int x2, int y2);

        /**
         * @brief   Draw horizontal line
         * @param   x       X coordinate or starting (left) point
         * @param   y       Y coordinate or starting (left) point
         * @param   w       Line width
         */
        void drawHorizontalLine(int x, int y, int x2);

        /**
         * @brief   Draw vertical line
         * @param   x       X coordinate or starting (top) point
         * @param   y       Y coordinate or starting (top) point
         * @param   h       Line height
         */
        void drawVerticalLine(int x, int y, int y2);

        /**
         * @brief   Draw a rectangle
         * @param   x       X coordinate or starting (top left) point
         * @param   y       Y coordinate or starting (top left) point
         * @param   w       Rectangle width
         * @param   h       Rectangle height
         */
        void drawRectangle(int x, int y, int x2, int y2);

        /**
         * @brief   Draw a filled rectangle
         * @param   x       X coordinate or starting (top left) point
         * @param   y       Y coordinate or starting (top left) point
         * @param   w       Rectangle width
         * @param   h       Rectangle height
         */
        void fillRectangle(int x, int y, int x2, int y2);

        /**
         * @brief   Draw a filled circle
         * @param   x0      X coordinate or center
         * @param   y0      Y coordinate or center
         * @param   r       Radius
         */
        void drawCircle(int x0, int y0, int r);

        /**
         * @brief   Draw a filled circle
         * @param   x0      X coordinate or center
         * @param   y0      Y coordinate or center
         * @param   r       Radius
         */
        void fillCircle(int x0, int y0, int r);

        /**
         * @brief   Set font for drawing
         * @return  Previously selected font
         * @param   font     Font
         */
        const font_t* setFont(const font_t* font);

        /**
         * @brief   Set font for drawing
         * @return  Current font
         */
        const font_t* font() const;

        /**
         * @brief   Select built-in font for drawing
         * @return  Previously selected font
         * @param   idx     Font index, see fonts.c
         */
        const font_t* setBuiltinFont(uint8_t idx);

        /**
         * @brief   Draw monochrome bitmap
         * @param   bitmap      Bitmap to be drawn
         * @param   x           X position (top-left corner)
         * @param   y           Y position (top-left corner)
         */
        void drawBitmap(const bitmap_t* bitmap, int x, int y, bool enable_alpha=true);

        /**
         * @brief   Draw one character using currently selected font
         * @param   x           X position of character (top-left corner)
         * @param   y           Y position of character (top-left corner)
         * @param   c           The character to draw
         * @return  Width of the character
         */
        int drawChar(int x, int y, int c);

        /**
         * @brief   Draw string using currently selected font
         * @param   x           X position of string (top-left corner)
         * @param   y           Y position of string (top-left corner)
         * @param   str         The string to draw
         * @return  Width of the string (out-of-display pixels also included)
         */
        int drawString(int x, int y, const std::string& str);

        /**
         * @brief   Draw string using currently selected font
         * @param   x           X position of string (top-left corner)
         * @param   y           Y position of string (top-left corner)
         * @param   str         The string to draw
         * @param   foreground  Character color
         * @param   background  Background color
         * @return  Width of the string (out-of-display pixels also included)
         */
        int drawString(int x, int y, const char* str);

        /**
         * @brief   Measure width of string with current selected font
         * @param   str         String to measure
         * @return  Width of the string
         */
        int measureString(const std::string& str);

        /**
         * @brief   Measure width of string with current selected font
         * @param   str         String to measure
         * @return  Width of the string
         */
        int measureString(const char* str);

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

    private:
        /**
         * @brief   Draw one pixel without clipping and dirty marking
         * @param   x       X coordinate
         * @param   y       Y coordinate
         * @param   color   Color of the pixel
         */
        void drawPixelRaw(int x, int y, color_t color);

        /**
         * @brief   Draw one pixel without dirty marking
         * @param   x       X coordinate
         * @param   y       Y coordinate
         * @param   color   Color of the pixel
         */
        void drawPixelClipped(int x, int y, color_t color);

        /*!
            @brief  Get mask of pixel
            @return Pixel bit mask
        */
        inline uint8_t getPixelMask(int y) const;

        /*!
            @brief  Get pixel offset
            @param  x
                    X coordinate of pixel
            @param  y
                    Y coordinate of pixel
            @return Pixel offset
        */
        inline uint16_t getPixelOffset(int x, int y) const;

    public:
        /*!
            @brief  Get display device interface
            @return Display device interface
        */
        Device* device();

    public:
        enum update_state_t {
            NO_UPDATE_NEEDED = 0x0,     //< no need to update display
            UPDATE_NEEDED = 0x1,        //< update needed, there are dirty pages
            FORCED_UPDATE = 0x2         //< full update of all pages requested
        };

        /**
         * @brief   Update display (send display buffer to the panel)
         *          The program automatically tracks "dirty" regions to minimize
         *          refresh area.
         */
        void update();

        /**
         * @brief   Refresh display (send display buffer to the panel)
         * @param   force   The program automatically tracks "dirty" regions to
         *          minimize refresh area. Set #force to true to ignore the dirty region and
         *          refresh the whole screen.
         */
        void update(bool force);

        /**
         * @brief   Perform actual device refresh. If deferred update is enabled,
         *          this is automatically done by the application after each update
         *          cycle.
         */
        void refresh();

        /**
         * @brief  Set update status
         * @param  update_state Update status
         * @return None (void).
         */
        void setUpdateState(update_state_t update_state);

        /**
         * @brief  Get update status
         * @return Update status
         */
        update_state_t getUpdateState() const;

        /**
         * @brief  Set deferred update mode. If deferred, the application updates the
         *         display after each update cycle in case dirty pages exist.
         * @param  deferred_update Enable or disable deferred update mode
         * @return None (void).
         */
        void setDeferredUpdate(bool deferred_update);

        /**
         * @brief  Get deferred update enable flag.
         * @return Deferred update enable flag.
         */
        bool getDeferredUpdate() const;

        /*!
            @brief  Enable partial update.
            @param  enable
                    Enable or disable partial updates to avoid display data transfer
            @return None (void).
        */
        void setPartialUpdate(bool enable = true);

        /*!
            @brief  Get partial update flag.
            @return Enable status of partial updates
        */
        bool getPartialUpdate() const;

        /*!
            @brief  Lock display memory page.
            @param  page
                    Memory page number
            @param  lock
                    Lock status on or off
            @return None (void).
        */
        void lockPage(int page, bool lock=true);


    private:
        Device* device_{nullptr};                               // display device;
        uint8_t width_{0};                                      // panel width (128)
        uint8_t height_{0};                                     // panel height (32 or 64)
        const font_t* font_{nullptr};                           // current font
        update_state_t update_state_{NO_UPDATE_NEEDED};         // update state
        bool deferred_update_{false};                           // deferred update
        color_t foreground_{WHITE};                             // foreground color
        color_t background_{BLACK};                             // background color

    public:
        Display(const Display&) = delete;
        Display(const Display&&) = delete;
        Display& operator=(const Display&) = delete;
        Display& operator=(const Display&&) = delete;
        ~Display() = default;

    public:
};

}  // namespace graphics
