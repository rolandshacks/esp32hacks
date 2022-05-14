//
// Bitmap
//
#pragma once

#include <cstdint>
#include <cstddef>

namespace graphics {

//! @brief Bitmap
class Bitmap {
    public:
        Bitmap(uint16_t width, uint16_t height, bool alpha_channel, uint16_t bytes_per_line,
               size_t size, const uint8_t* pixels=nullptr);
        Bitmap(uint16_t width, uint16_t height, uint16_t bits_per_pixel, const uint8_t* pixels=nullptr);
        ~Bitmap();

    public:
        inline uint16_t width() const { return width_; }
        inline uint16_t height() const { return height_; }
        inline bool hasAlpha() const { return alpha_channel_; }
        inline uint16_t bytesPerLine() const { return bytes_per_line_; }
        inline uint16_t bitsPerPixel() const { return bits_per_pixel_; }
        inline size_t size() const { return size_; }
        inline const void* pixels() const { return pixels_ref_; }
        inline const uint8_t* getPixelBytes() const { return pixels_ref_; }

    public:
        void* lock();
        void unlock();

    private:
        void alloc(const uint8_t* pixel_ref);
        void free();

    private:
        uint16_t width_;                            //!< Bitmap width
        uint16_t height_;                           //!< Bitmap height
        bool alpha_channel_;                        //!< True if alpha channel exists (second byte)
        uint16_t bytes_per_line_;                   //!< Number of bytes per line
        uint16_t bits_per_pixel_;                   //!< Number of bits per pixel
        size_t size_;                               //!< Bitmap size in bytes
        uint8_t* pixels_;                           //!< Pointer to bitmap data
        const uint8_t* pixels_ref_;                 //!< Pointer to bitmap data reference
        bool allocated_;                            //!< Indicates bitmap data is dynamically allocated
        bool locked_;                               //!< Indicates bitmap data is locked for write

    private:
        Bitmap() = delete;
        Bitmap(const Bitmap&) = delete;
        Bitmap(const Bitmap&&) = delete;
        Bitmap& operator=(const Bitmap&) = delete;
        Bitmap& operator=(Bitmap&&) = delete;
};

}  // namespace
