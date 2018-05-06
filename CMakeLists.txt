cmake_minimum_required(VERSION 3.5.1)
project(raspi)

# set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -std=c99 -Wall -Wno-discarded-qualifiers")
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wall -g -Wno-discarded-qualifiers")

link_directories(/home/tobi/qtcreator/sdl1.2-debug/lib /home/tobi/qtcreator/sdl_ttf/lib)
include_directories(/home/tobi/qtcreator/sdl1.2-debug/include /home/tobi/qtcreator/sdl_ttf/include /home/tobi/qtcreator/sdl1.2-debug/include/SDL)
set(CMAKE_BUILD_TYPE Debug)
set( CMAKE_VERBOSE_MAKEFILE on )
add_subdirectory(library)
add_subdirectory(test)
