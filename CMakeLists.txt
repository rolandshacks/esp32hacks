#
# Build Configuration
#
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(oleddemo)
