#
# Build Configuration
#
idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "include" "lib/oled/include" "lib/app/include"
)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Import libraries
add_subdirectory(lib/oled)
add_subdirectory(lib/app)

# Link libraries to main component
target_link_libraries(${COMPONENT_LIB} PUBLIC lib_app lib_oled)
