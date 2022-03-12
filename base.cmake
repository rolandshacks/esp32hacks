#
# Common build setup
#
macro(setup_project)

    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

    include($ENV{IDF_PATH}/tools/cmake/project.cmake)

    message("CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")

    list(APPEND EXTRA_COMPONENT_DIRS
        ${CMAKE_SOURCE_DIR}/../libs/utils
        ${CMAKE_SOURCE_DIR}/../libs/app
        ${CMAKE_SOURCE_DIR}/../libs/oled
    )

    get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    string(REPLACE " " "_" PROJECT_NAME ${PROJECT_NAME})

#    project(${PROJECT_NAME} C CXX)

endmacro()
