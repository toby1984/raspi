cmake_minimum_required(VERSION 3.5.1)
project(ui)
add_library(ui OBJECT src/ui.c)
set_target_properties(ui PROPERTIES COMPILE_FLAGS -fPIC)
