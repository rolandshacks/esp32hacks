
idf_component_register(
    SRCS "src/base.cpp" "src/device.cpp" "src/bitmap.cpp" "src/display.cpp" "src/fonts.cpp" "src/oscilloscope.cpp" "src/font_glcd_5x7.inc" "src/font_tahoma_8pt.inc" "src/font_ubuntu_6pt.inc" "src/font_game_12pt.inc"
    INCLUDE_DIRS "include" "${IDF_PATH}/components/driver/include"
    REQUIRES sys
)
