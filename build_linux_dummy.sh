#!/bin/bash

## Dependencies

# install dependencies
if [[ " $@ " =~ " --dependencies " ]]; then
	if [[ -f /etc/debian_version ]]; then
		# Debian/Ubuntu/Raspbian
		sudo apt install -y g++
	elif [[ -f /etc/arch-release ]]; then
		# Arch/Manjaro
		sudo pacman -S --noconfirm base-devel
	else
		echo "unable to install dependencies - unsupported system"
	fi
fi


## Prepare

# construct version string
BUILD="HUI-linux-$(uname -m)-dummy"

# create target directory 
mkdir -p $BUILD
cd $BUILD


## Build

# build
g++ -shared -fPIC -o ./libHUI.so ../hui_webview__dummy.cc -I..


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


cd ..