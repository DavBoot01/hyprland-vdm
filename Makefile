.PHONY: all build install clean uninstall configure

all: build

configure:
	cmake -B build

build: configure
	cmake --build build

install: build
	cmake --install build

load: build
	hyprctl plugin load ${PWD}/build/libhyprland-vdm.so

unload:
	hyprctl plugin unload ${PWD}/build/libhyprland-vdm.so

clean:
	rm -rf build

uninstall:
	rm -f ~/.config/hypr/plugins/libhyprland-vdm.so

rebuild: clean build
