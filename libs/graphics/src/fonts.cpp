//
// Fonts
//
#include "graphics/base.h"

namespace graphics {

#include "font_glcd_5x7.inc"
#include "font_tahoma_8pt.inc"
#include "font_ubuntu_6pt.inc"
#include "font_game_12pt.inc"

const Font* BUILTIN_FONTS[] = {
    &glcd_5x7_font_info,
    &tahoma_8pt_font_info,
    &ubuntu_12pt_font_info,
    &game_12pt_font_info
};

const size_t BUILTIN_FONT_COUNT = sizeof(BUILTIN_FONTS) / sizeof(BUILTIN_FONTS[0]);

}  // namespace
