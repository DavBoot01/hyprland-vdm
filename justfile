plugin_name := "libhyprland-vdm"
plugin_so   := justfile_directory() / "build" / plugin_name + ".so"

default: build

configure:
    cmake -B build

build: configure
    cmake --build build

install: build
    cmake --install build

uninstall:
    rm -f ~/.config/hypr/plugins/{{plugin_name}}.so

load: build
    hyprctl plugin load {{plugin_so}}

unload:
    hyprctl plugin unload {{plugin_so}}

reload: build
    hyprctl plugin unload {{plugin_so}} || true
    hyprctl plugin load {{plugin_so}}

test: reload
    hyprctl vdminfo
    hyprctl printmonitorinfo 0
    hyprctl printmonitorinfo 1
    hyprctl printmonitorinfo 2

# Run clang-tidy on all sources (uses a separate build dir to avoid
# polluting the normal build cache with the extra clang-tidy overhead).
tidy:
    cmake -B build-tidy -DENABLE_CLANG_TIDY=ON
    cmake --build build-tidy

clean:
    rm -rf build

rebuild: clean build
