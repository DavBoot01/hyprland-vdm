.PHONY: all build install clean uninstall configure

all: build

configure:
	cmake -B build

build: configure
	cmake --build build

install: build
	cmake --install build

clean:
	rm -rf build

uninstall:
	rm -f ~/.config/hypr/plugins/libhyprland-vdm.so

rebuild: clean build
