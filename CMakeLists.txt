cmake_minimum_required(VERSION 3.10)
project(sim C CXX)

###################################################################################################
# Specify the application class name of the example to be started:
# Hello HelloGraphics BitmapDemo ScrollingDemo OscilloscopeDemo BoingBall Graphics3D RetroDemo
set (ACTIVE_EXAMPLE Graphics3D)
###################################################################################################

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Xlinker /SUBSYSTEM:CONSOLE")

###################################################################################################

add_definitions(-DSDL_MAIN_HANDLED)
add_compile_definitions(SIMULATOR)
add_compile_definitions(SIMULATOR_APP=${ACTIVE_EXAMPLE})

###################################################################################################

set(SDL2_INCLUDE_DIR C:/tools/sdk/sdl/include)
set(SDL2_LIBRARY C:/tools/sdk/sdl/lib/x64)

###################################################################################################

include_directories(
    "libs/sim/include/mockup"
    "libs/sim/include"
    "libs/sys/include"
    "libs/graphics/include"
    "libs/graphics3d/include"
    "libs/application/include"
    ${SDL2_INCLUDE_DIR}
)

file (GLOB LIBHDR
    "libs/sim/include/*.h"
    "libs/sys/include/sys/*.h"
    "libs/graphics/include/graphics/*.h"
    "libs/graphics/src/font_*.inc"
    "libs/graphics/include/graphics3d/*.h"
    "libs/application/include/applicaion/*.h"
)

set (LIBSRC
    "libs/graphics/src/base.cpp"
    "libs/graphics/src/fonts.cpp"
    "libs/graphics/src/bitmap.cpp"
    "libs/graphics/src/device.cpp"
    "libs/graphics/src/display.cpp"
    "libs/graphics/src/oscilloscope.cpp"
    "libs/graphics3d/src/base.cpp"
    "libs/graphics3d/src/renderer.cpp"
    "libs/application/src/application.cpp"
    "libs/sys/src/i2c.cpp"
    "libs/sim/src/adc.cpp"
    "libs/sim/src/freertos.cpp"
    "libs/sim/src/log.cpp"
    "libs/sim/src/main.cpp"
    "libs/sim/src/sim.cpp"
    "libs/sim/src/ssd1306.cpp"
    "libs/sim/src/sys.cpp"
)

###################################################################################################

set (ALL_EXAMPLE_MAINS
    hello/main/main.cpp
    hello_graphics/main/main.cpp
    bitmaps/main/main.cpp
    boing_ball/main/main.cpp
    oscilloscope/main/main.cpp
    scrolling/main/main.cpp
    3ddemo/main/main.cpp
    retro_demo/main/main.cpp
)

set(SRC ${LIBHDR} ${LIBSRC} ${ALL_EXAMPLE_MAINS})

###################################################################################################

add_executable (${PROJECT_NAME} ${SRC})

###################################################################################################

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC
    #${SDL2_LIBRARY}/SDL2main.lib
    ${SDL2_LIBRARY}/SDL2.lib
    winmm.lib
    ws2_32.lib
    mswsock
    advapi32
)
