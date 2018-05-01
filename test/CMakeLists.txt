cmake_minimum_required(VERSION 3.5.1)
project(test LANGUAGES C)

include_directories(../library/src)
add_executable(test_sdl src/test.c)

link_directories(../bin/library)

target_link_libraries(test_sdl mylib)
