//
// Base types and definitions
//
#pragma once

#include <cstdint>
#include <cstddef>

namespace graphics {

//! @brief Drawing color
typedef enum {
    TRANSPARENT = -1,  //!< Transparent (not drawing)
    BLACK = 0,         //!< Black (pixel off)
    WHITE = 1,         //!< White (or blue, yellow, pixel on)
    INVERT = 2,        //!< Invert pixel (XOR)
} Color;

//! @brief Point coordinate
class Point2 {
   public:
    int x;
    int y;

   public:
    Point2() : x(0), y(0) { ; }
    Point2(int _x, int _y) : x(_x), y(_y) { ; }

    inline void set(int _x, int _y) {
        x = _x; y = _y;
    }
};

//! @brief Display page information
class Page {
   public:
    int dirty_left;
    int dirty_right;
    bool lock;

    Page() : dirty_left(0), dirty_right(0), lock(false) { ; }
};

class Rectangle {
   public:
    Rectangle()
        : left(0), right(0), top(0), bottom(0) {}

    Rectangle(int l, int r, int t, int b)
        : left(l), right(r), top(t), bottom(b) {}

    Rectangle(const Rectangle& r)
        : left(r.left), right(r.right), top(r.top), bottom(r.bottom) {}

   public:
    inline void set(int l, int r, int t, int b) {
        left = l;
        right = r;
        top = t;
        bottom = b;
    }

    inline void set(const Rectangle& r) {
        set(r.left, r.right, r.top, r.bottom);
    }

    inline int width() const {
        return right - left;
    }

    inline int height() const {
        return bottom - top;
    }

    void join(int x, int y) {
        if (left > x) left = x;
        if (right < x) right = x;
        if (top > y) top = y;
        if (bottom < y) bottom = y;
    }

    void join(int l, int r, int t, int b) {
        if (left > l) left = l;
        if (right < r) right = r;
        if (top > t) top = t;
        if (bottom < b) bottom = b;
    }

    void clip(int l, int r, int t, int b) {
        if (left < l) left = l;
        if (left > r) left = r;
        if (right > r) right = r;
        if (right < left) right = left;

        if (top < t) top = t;
        if (top > b) top = b;
        if (bottom > b) bottom = b;
        if (bottom < top) bottom = top;
    }

    void clip(const Rectangle& r) {
        clip(r.left, r.right, r.top, r.bottom);
    }

    bool is_valid() const {
        return (top <= bottom) && (left <= right);
    }

    void normalize();

   public:
    int left;
    int right;
    int top;
    int bottom;
};

//! @brief Character descriptor
typedef struct _font_char_desc {
    uint8_t width;    //!< Character width in pixel
    uint16_t offset;  //!< Offset of this character in bitmap
} FontCharDescriptor;

//! @brief Font information
typedef struct _font {
    uint8_t height;                              //!< Character height in pixel, all characters have same height
    uint8_t c;                                   //!< Simulation of "C" width in TrueType term, the space between adjacent characters
    uint8_t char_start;                          //!< First character
    uint8_t char_end;                            //!< Last character
    const FontCharDescriptor* char_descriptors;  //!< Descriptor for each character
    const uint8_t* bitmap;                       //!< Character bitmap
} Font;

extern const Font* BUILTIN_FONTS[];
extern const size_t BUILTIN_FONT_COUNT;

template <typename T> void sort_pair(T& a, T& b) {
    if (a > b) {
        T c{a};
        a=b; b=c;
    }
}

}  // namespace
