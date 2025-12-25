# Copilot Instructions for hyprland-vdm

## Project Overview
Minimal C++ plugin skeleton for Hyprland. Currently contains only the basic plugin structure for compilation testing.

## Architecture & Key Components

### Core Files
- **[src/main.cpp](src/main.cpp)**: Plugin entry point with `PLUGIN_INIT()`, `PLUGIN_EXIT()`, and `PLUGIN_API_VERSION()`
- **[include/globals.hpp](include/globals.hpp)**: Global constants (plugin name, version, author)
- **[CMakeLists.txt](CMakeLists.txt)**: CMake build configuration
- **[Makefile](Makefile)**: Convenience wrapper for CMake commands

## Development Conventions

### Language & Build
- **C++23** with `-std=c++23` flag (Hyprland requirement)
- **Compiler**: GCC 13+ or Clang 16+
- **Build**: `cmake -B build && cmake --build build` compiles to `build/libhyprland-vdm.so`, `cmake --install build` copies to `~/.config/hypr/plugins/`
- Shared library compiled with `-shared -fPIC`

### Hyprland Plugin API Basics
```cpp
// Required exports
PLUGIN_API_VERSION()        // Must match HYPRLAND_API_VERSION
PLUGIN_INIT(HANDLE handle)  // Initialization, return description info
PLUGIN_EXIT()               // Cleanup

// Notifications (uses CHyprColor with r, g, b, a)
HyprlandAPI::addNotification(handle, "message", CHyprColor(0.2, 0.8, 0.2, 1.0), duration_ms);
```

## Building & Testing

```bash
# Development cycle
make                              # Configure and build
make install                      # Install to ~/.config/hypr/plugins/
hyprctl reload                    # Reload Hyprland config (loads plugin)

# Clean rebuild
make clean && make                # or: make rebuild

# Direct CMake usage (alternative)
cmake -B build                    # Configure build
cmake --build build               # Build plugin
cmake --install build             # Install to user config

# Verify plugin loads
journalctl -f --user-unit hyprland  # Watch for "[VDM] Plugin loaded" message
```

## External Dependencies
- **hyprland-devel**: Headers from `/usr/include/hyprland/` (protocols, wlroots, pixman)
- **pkg-config**: Resolve Hyprland compile/link flags
- No runtime dependencies beyond Hyprland itself

## Important Notes
- **API Version**: Must export `PLUGIN_API_VERSION()` matching Hyprland's `HYPRLAND_API_VERSION`
- **ABI Compatibility**: Rebuild plugin after Hyprland updates (C++ ABI fragility)
- **Plugin Loading**: Load in `hyprland.conf` with `plugin = ~/.config/hypr/plugins/hyprland-vdm.so`
