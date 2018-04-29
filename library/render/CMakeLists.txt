cmake_minimum_required(VERSION 3.5.1)
project(render)
add_library(render OBJECT src/render.c)
set_target_properties(render PROPERTIES COMPILE_FLAGS -fPIC)
