//
// Base
//
#include "graphics/base.h"

#include <utility>

using namespace graphics;

void Rectangle::normalize() {
    if (left > right) std::swap(left, right);
    if (top > bottom) std::swap(top, bottom);
}
