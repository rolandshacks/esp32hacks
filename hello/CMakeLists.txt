#
# Project configuration
#
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

list(APPEND EXTRA_COMPONENT_DIRS
    ${CMAKE_SOURCE_DIR}/../libs/utils
    ${CMAKE_SOURCE_DIR}/../libs/app
    ${CMAKE_SOURCE_DIR}/../libs/oled
)

project(oleddemo)
