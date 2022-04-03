//
// Base types and definitions
//
#pragma once

#include <cstdint>

namespace graphics {

//! @brief Drawing color
typedef enum {
    TRANSPARENT = -1,  //!< Transparent (not drawing)
    BLACK = 0,         //!< Black (pixel off)
    WHITE = 1,         //!< White (or blue, yellow, pixel on)
    INVERT = 2,        //!< Invert pixel (XOR)
} color_t;

class Point {
   public:
    int8_t x;
    int8_t y;

    Point() : x(0), y(0) { ; }
};

class Page {
   public:
    int dirty_left;
    int dirty_right;
    bool lock;

    Page() : dirty_left(0), dirty_right(0), lock(false) { ; }
};

class Rectangle {
   public:
    Rectangle() : left(0), right(0), top(0), bottom(0) {}
    Rectangle(uint8_t l, uint8_t r, uint8_t t, uint8_t b)
        : left(l), right(r), top(t), bottom(b) {}
    Rectangle(const Rectangle& r)
        : left(r.left), right(r.right), top(r.top), bottom(r.bottom) {}

   public:
    void set(uint8_t l, uint8_t r, uint8_t t, uint8_t b) {
        left = l;
        right = r;
        top = t;
        bottom = b;
    }

    void set(const Rectangle& r) { set(r.left, r.right, r.top, r.bottom); }

    void join(uint8_t x, uint8_t y) {
        if (left > x) left = x;
        if (right < x) right = x;
        if (top > y) top = y;
        if (bottom < y) bottom = y;
    }

    void join(uint8_t l, uint8_t r, uint8_t t, uint8_t b) {
        if (left > l) left = l;
        if (right < r) right = r;
        if (top > t) top = t;
        if (bottom < b) bottom = b;
    }

    void clip(uint8_t l, uint8_t r, uint8_t t, uint8_t b) {
        if (left < l) left = l;
        if (right > r) right = r;
        if (top < t) top = t;
        if (bottom > b) bottom = b;
    }

    void invalidate() { set(255, 0, 255, 0); }

    bool is_valid() const { return (top <= bottom) && (left <= right); }

   public:
    uint8_t left;
    uint8_t right;
    uint8_t top;
    uint8_t bottom;
};

}  // namespace graphics
