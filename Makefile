.PHONY: all build install clean uninstall configure load unload reload rebuild

PLUGIN_NAME=libhyprland-vdm

all: build

configure:
	cmake -B build

build: configure
	cmake --build build

install: build
	cmake --install build

load: build
	hyprctl plugin load $(shell pwd)/build/$(PLUGIN_NAME).so

unload:
	hyprctl plugin unload $(shell pwd)/build/$(PLUGIN_NAME).so

reload:build
	hyprctl plugin unload $(shell pwd)/build/$(PLUGIN_NAME).so || true
	hyprctl plugin load $(shell pwd)/build/$(PLUGIN_NAME).so

test: reload
	hyprctl vdminfo

clean:
	rm -rf build

uninstall:
	rm -f ~/.config/hypr/plugins/libhyprland-vdm.so

rebuild: clean build
