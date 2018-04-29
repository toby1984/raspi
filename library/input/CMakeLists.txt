cmake_minimum_required(VERSION 3.5.1)
project(input)
add_library(input OBJECT src/input.c)
set_target_properties(input PROPERTIES COMPILE_FLAGS -fPIC)
