cmake_minimum_required(VERSION 3.5.1)
project(test LANGUAGES C)
link_directories(../bin/library)

find_library(lib1_location mylib)
message("mylib is at: ${lib1_location}")
target_link_libraries(mylib LINK_PUBLIC ${lib1_location})

include_directories(../library/src)
add_executable(test_sdl src/test.c)
