
idf_component_register(
    SRCS "src/display.cpp" "src/font_glcd_5x7.inc" "src/font_tahoma_8pt.inc" "src/font_ubuntu_6pt.inc" "src/font_game_12pt.inc" "src/fonts.cpp" "src/oscilloscope.cpp"
    INCLUDE_DIRS "include" "${IDF_PATH}/components/driver/include"
    REQUIRES sys
)
