cmake_minimum_required(VERSION 3.19)
project(hyprland-vdm
    VERSION 0.1.0
    DESCRIPTION "Virtual Desktop Manager for Hyprland"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Optional clang-tidy static analysis.
# Enable with: cmake -B build -DENABLE_CLANG_TIDY=ON
option(ENABLE_CLANG_TIDY "Run clang-tidy on project sources during build" OFF)
if (ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy REQUIRED
        DOC "Path to clang-tidy executable")
    message(STATUS "clang-tidy enabled: ${CLANG_TIDY_EXE}")
    # --header-filter restricts diagnostics to our own headers only;
    # Hyprland/system headers are marked SYSTEM and are already suppressed by
    # clang-tidy, but the filter makes that boundary explicit.
    set(CLANG_TIDY_CMD
        "${CLANG_TIDY_EXE}"
        "--header-filter=${CMAKE_SOURCE_DIR}/include/.*"
    )
endif()

# Find Hyprland
find_package(PkgConfig REQUIRED)
pkg_check_modules(HYPRLAND REQUIRED hyprland)
pkg_check_modules(HYPRUTILS REQUIRED hyprutils)
pkg_check_modules(DRM REQUIRED libdrm)

# Find nlohmann/json
find_package(nlohmann_json 3.11.0 REQUIRED)

# Plugin source files
add_library(${PROJECT_NAME} MODULE
    src/main.cpp
    src/commands.cpp
    src/HyprlandUtils.cpp
    src/LoggerFacade.cpp
    src/VirtualDesktop.cpp
    src/Layout.cpp
    src/VirtualDesktopManager.cpp
)

# nlohmann/json integration (prefer system package, fallback to header-only)
if (nlohmann_json_FOUND)
    target_link_libraries(
        ${PROJECT_NAME} 
        PRIVATE 
        nlohmann_json::nlohmann_json
    )
else()
    message(STATUS "nlohmann_json not found, falling back to header-only include in ${CMAKE_SOURCE_DIR}/include")
    target_include_directories(
        ${PROJECT_NAME} 
        PRIVATE 
        ${CMAKE_SOURCE_DIR}/include
    )
endif()

# Wire up clang-tidy if enabled (set on target, not globally, to avoid
# running it on FetchContent or other auto-generated targets).
if (ENABLE_CLANG_TIDY)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_CLANG_TIDY "${CLANG_TIDY_CMD}"
    )
endif()

# Compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -g
)

# Include directories (our own headers)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# System include directories (suppress warnings from external headers)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
    ${HYPRLAND_INCLUDE_DIRS}
    ${HYPRUTILS_INCLUDE_DIRS}
    ${DRM_INCLUDE_DIRS}
    /usr/include/hyprland/protocols
    /usr/include/hyprland/wlroots
    /usr/include/pixman-1
)

# Link options
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${HYPRLAND_LIBRARIES}
    ${HYPRUTILS_LINK_LIBRARIES}
)

# Installation
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION $ENV{HOME}/.config/hypr/plugins
)
