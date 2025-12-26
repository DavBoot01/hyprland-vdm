cmake_minimum_required(VERSION 3.19)
project(hyprland-vdm
    VERSION 0.1.0
    DESCRIPTION "Virtual Desktop Manager for Hyprland"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find Hyprland
find_package(PkgConfig REQUIRED)
pkg_check_modules(HYPRLAND REQUIRED hyprland)

# Plugin source files
add_library(hyprland-vdm MODULE
    src/main.cpp
    src/commands.cpp
)

# Compiler flags
target_compile_options(hyprland-vdm PRIVATE
    -Wall
    -Wextra
    -g
)

# Include directories (our own headers)
target_include_directories(hyprland-vdm PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# System include directories (suppress warnings from external headers)
target_include_directories(hyprland-vdm SYSTEM PRIVATE
    ${HYPRLAND_INCLUDE_DIRS}
    /usr/include/hyprland/protocols
    /usr/include/hyprland/wlroots
    /usr/include/pixman-1
)

# Link options
target_link_libraries(hyprland-vdm PRIVATE
    ${HYPRLAND_LIBRARIES}
)

# Installation
install(TARGETS hyprland-vdm
    LIBRARY DESTINATION $ENV{HOME}/.config/hypr/plugins
)
