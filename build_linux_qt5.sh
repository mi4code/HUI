#!/bin/bash

## Dependencies

# install dependencies
if [[ " $@ " =~ " --dependencies " ]]; then
	if [[ -f /etc/arch-release ]]; then
		# Arch/Manjaro
		yay -S --noconfirm qt5-webkit
	else
		echo "unable to install dependencies - unsupported system"
		#sudo apt install -y libqt5webkit5
	fi
fi


## Prepare

# construct version string
BUILD="HUI-linux-$(uname -m)-webkit-qt5"

# create target directory 
mkdir -p $BUILD
cd $BUILD


## Build

# build
g++ -shared -fPIC -o ./libHUI.so ../hui_webview__webkit_qt5.cc -I.. `pkg-config --cflags --libs Qt5Widgets Qt5WebKitWidgets`

# TODO: qmake *.moc


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

# Qt uses moc (Meta-Object Compiler) so you may need to use `qmake` (and `make`) in order to generate `qt5_generated.moc` (pre-generated one is already present in the source tree - just in case you need to generate it, there is guide):
#  ```
#  cd .
#  qmake ./qt5_build.pro
#  make
#  mv ./release/*.moc ./qt5_generated.moc
#  ```
#  (note: make doesnt have to complete compilation - we just need it to generate that one file)


cd ..