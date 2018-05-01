cmake_minimum_required(VERSION 3.5.1)
project(mylib VERSION 1.0.1 LANGUAGES C)
include(GNUInstallDirs)

# add_library(mylib SHARED src/mylib.c $<TARGET_OBJECTS:ui> $<TARGET_OBJECTS:input> $<TARGET_OBJECTS:render>)
add_library(mylib SHARED src/mylib.c src/input.c src/render.c  src/ui.c)

target_link_libraries(mylib SDL SDL_ttf pthread)

set_target_properties(mylib PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER src/mylib.h)
configure_file(mylib.pc.in mylib.pc @ONLY)
target_include_directories(mylib PRIVATE .)
install(TARGETS mylib
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_BINARY_DIR}/mylib.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
