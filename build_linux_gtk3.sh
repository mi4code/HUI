#!/bin/bash

## Dependencies

# install dependencies
if [[ " $@ " =~ " --dependencies " ]]; then
	if [[ -f /etc/debian_version ]]; then
		# Debian/Ubuntu/Raspbian
		sudo apt install -y g++ libgtk-3-dev libwebkit2gtk-4.0-dev libgtk-layer-shell-dev
	elif [[ -f /etc/arch-release ]]; then
		# Arch/Manjaro
		sudo pacman -S --noconfirm base-devel gtk3 webkit2gtk gtk-layer-shell
	else
		echo "unable to install dependencies - unsupported system"
	fi
fi


## Prepare

# construct version string
BUILD="HUI-linux-$(uname -m)-webkit-gtk3"

# create target directory 
mkdir -p $BUILD
cd $BUILD


## Build

# build
g++ -shared -fPIC -o ./libHUI.so ../hui_webview__webkit_gtk3.cc -I.. `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0 gtk-layer-shell-0`


## Deploy

if [[ " $@ " =~ " --deploy " ]]; then
	# TODO: linux packaging; AUR package, pkg config file
	echo "deploying not supported"
fi


## Tests
if [[ " $@ " =~ " --tests " ]]; then
	# build tests
	g++ -o test_webview_js_api ../tests/test_webview_js_api.cc -I.. -L. -lHUI
fi


## Notes

# if you got too old gtk layer shell, build it from source
# (rpi1b) if there is error "undefined reference to ...", add `--std=c++17`
# all-in-one build: g++ -o <your_executable> <your_source> `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0 gtk-layer-shell-0`
# TODO: upgrade libwebkit2gtk-4.0-dev to libwebkit2gtk-4.1-dev


cd ..