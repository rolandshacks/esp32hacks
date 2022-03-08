#
# Build Configuration
#
cmake_minimum_required(VERSION 3.10)

project(lib_oled)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set (SOURCE
    src/i2c.cpp
    src/ssd1306.cpp
    src/font_glcd_5x7.cpp
    src/font_tahoma_8pt.cpp
    src/fonts.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(${PROJECT_NAME} ${SOURCE})
