//
// Bitmap
//
#include "graphics/base.h"
#include "graphics/bitmap.h"

#include <cassert>

using namespace graphics;

Bitmap::Bitmap(uint16_t width, uint16_t height,
               bool alpha_channel, uint16_t bytes_per_line, size_t size,
               const uint8_t* pixels)
    : width_(width),
      height_(height),
      alpha_channel_(alpha_channel),
      bytes_per_line_(bytes_per_line),
      bits_per_pixel_(alpha_channel ? 2 : 1),
      size_(size),
      allocated_(false),
      locked_(false) {
    alloc(pixels);
}

Bitmap::Bitmap(uint16_t width, uint16_t height, uint16_t bits_per_pixel, const uint8_t* pixels) :
    width_(width), height_(height),
    alpha_channel_(false),
    bytes_per_line_(width * bits_per_pixel / 8),
    bits_per_pixel_(bits_per_pixel),
    allocated_(false),
    locked_(false)
{
    size_ = height_ * bytes_per_line_;
    alloc(pixels);
}

Bitmap::~Bitmap() {
    free();
}

void Bitmap::alloc(const uint8_t* pixel_ref) {
    if (nullptr != pixel_ref) {
        allocated_ = false;
        pixels_ref_ = pixel_ref;
        pixels_ = nullptr;
    } else {
        allocated_ = true;
        pixels_ = new uint8_t[size_];
        pixels_ref_ = pixels_;
    }
}

void Bitmap::free() {
    if (allocated_ && nullptr != pixels_) {
        delete [] pixels_;
    }
    allocated_ = false;
    pixels_ = nullptr;
    pixels_ref_ = nullptr;
}

void* Bitmap::lock() {
    assert(false == locked_);
    assert(nullptr != pixels_);
    locked_ = true;
    return pixels_;
}

void Bitmap::unlock() {
    locked_ = false;
}
