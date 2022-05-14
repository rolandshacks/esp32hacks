#
# Common build setup
#
macro(setup_project)

	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

    include($ENV{IDF_PATH}/tools/cmake/project.cmake)

    message("CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")

    list(APPEND EXTRA_COMPONENT_DIRS
        ${CMAKE_SOURCE_DIR}/../libs/sys
        ${CMAKE_SOURCE_DIR}/../libs/graphics
        ${CMAKE_SOURCE_DIR}/../libs/graphics3d
        ${CMAKE_SOURCE_DIR}/../libs/application
    )

    get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    string(REPLACE " " "_" PROJECT_NAME ${PROJECT_NAME})

#    project(${PROJECT_NAME} C CXX)

endmacro()
