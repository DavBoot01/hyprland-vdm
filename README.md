# hyprland-vdm

Virtual Desktop Manager (VDM) plugin skeleton for Hyprland - a minimal C++ plugin template for rapid development.

## Overview

This is a minimal, ready-to-compile Hyprland plugin skeleton using C++23 and CMake. It demonstrates the basic plugin structure with proper API integration and serves as a starting point for custom Hyprland extensions.

## Features

- ✅ Minimal working plugin structure
- ✅ C++23 with CMake build system
- ✅ Makefile wrapper for convenient commands
- ✅ Plugin lifecycle management (init/exit)
- ✅ Notification system integration
- ✅ **CWorkspaceManager singleton** for workspace/monitor management
- ✅ Ready for extension with custom dispatchers and event handlers

## Building

### Prerequisites

- **Hyprland development headers** (`hyprland` package or built from source)
- **C++23 compatible compiler** (GCC 13+, Clang 16+)
- **CMake** 3.19 or newer
- **pkg-config** for dependency resolution

### Quick Start

```bash
# Build the plugin
make

# Install to Hyprland plugins directory
make install

# Reload Hyprland to load the plugin
hyprctl reload
```

### Build Targets

```bash
make                # Configure and build
make install        # Build and install to ~/.config/hypr/plugins/
make clean          # Remove build directory
make rebuild        # Clean and build from scratch
make uninstall      # Remove installed plugin
```

### Manual CMake Build

If you prefer direct CMake usage:

```bash
cmake -B build           # Configure
cmake --build build      # Compile
cmake --install build    # Install
```

## Installation

The plugin is installed to `~/.config/hypr/plugins/libhyprland-vdm.so`.

Add to your `hyprland.conf`:

```conf
# Load the VDM plugin
plugin = ~/.config/hypr/plugins/libhyprland-vdm.so
```

Then reload Hyprland:

```bash
hyprctl reload
```

## Verification

Check that the plugin loaded successfully:

```bash
# Watch Hyprland logs for the "[VDM] Plugin loaded" message
journalctl -f --user-unit hyprland

# Or check the notification that appears when Hyprland starts
```

## Project Structure

```
hyprland-vdm/
├── src/
│   └── main.cpp              # Plugin entry point
├── include/
│   └── globals.hpp           # Global constants (name, version, author)
├── CMakeLists.txt            # CMake configuration
├── Makefile                  # Convenience wrapper
├── hyprland.toml             # Plugin metadata
└── .github/
    └── copilot-instructions.md   # Development guidelines
```

## Development

This skeleton provides the foundation for building custom Hyprland functionality:

- **Add custom dispatchers**: Register new commands via `HyprlandAPI::addDispatcher()`
- **Hook into events**: Subscribe to workspace/window events with `HyprlandAPI::registerCallbackDynamic()`
- **Add config options**: Define plugin settings via `HyprlandAPI::addConfigValue()`
- **Access compositor state**: Use `g_pCompositor` API for workspace/window manipulation

See [.github/copilot-instructions.md](.github/copilot-instructions.md) for detailed development guidelines, API patterns, and best practices.

## Compatibility

- **Hyprland version**: 0.52+ (API changes may require updates)
- **ABI compatibility**: Rebuild after Hyprland updates due to C++ ABI sensitivity
- **API version**: Automatically matches `HYPRLAND_API_VERSION` at compile time

## Contributing

This is a minimal skeleton - fork and extend it for your needs! Common additions:

- Virtual desktop switching logic
- Workspace management features
- Custom keybind handlers
- Window manipulation utilities
- Integration with external tools

## License

BSD 3-Clause License - see [LICENSE](LICENSE) for details. This license is fully compatible with Hyprland (BSD 3-Clause).

## Acknowledgments

This work was inspired by [virtual-desktops](https://github.com/levnikmyskin/hyprland-virtual-desktops), which provided valuable insights into Hyprland plugin development.
