//
// Bitmaps data structures
//
#pragma once

#include <cstddef>
#include <cstdint>

namespace graphics {

//! @brief Bitmap information
typedef struct _bitmap_info {
    uint16_t width;                             //!< Bitmap width
    uint16_t height;                            //!< Bitmap height
    bool alpha_channel;                         //!< True if alpha channel exists (second byte)
    uint16_t bytes_per_line;                    //!< Number of bytes per line
    size_t size;                                //!< Bitmap size in bytes
    const uint8_t* pixels;                      //!< Pointer to bitmap data
} bitmap_t;

}  // namespace graphics
