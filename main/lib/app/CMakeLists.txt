#
# Build Configuration
#
cmake_minimum_required(VERSION 3.10)

project(lib_app)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set (SOURCE
    src/application.cpp
    src/oscilloscope.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../oled/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(${PROJECT_NAME} ${SOURCE})
