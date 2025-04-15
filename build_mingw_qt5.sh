# Start of MSYS2 shell script


## Dependencies

if [[ " $@ " =~ " --dependencies " ]]; then
	# build tools
	pacman -S --noconfirm --needed base-devel gcc pkg-config
	# qt5
	pacman -S --noconfirm --needed mingw-w64-x86_64-qt5 mingw-w64-x86_64-qt5-webview mingw-w64-x86_64-qtwebkit  mingw-w64-x86_64-kf5 mingw-w64-x86_64-kio-qt5 mingw-w64-x86_64-kparts-qt5 mingw-w64-x86_64-ninja mingw-w64-x86_64-cc mingw-w64-x86_64-qt5-tools
	#pacman -S --needed mingw-w64-x86_64-qt5 mingw-w64-x86_64-qt5-webview mingw-w64-x86_64-kdewebkit-qt5 mingw-w64-x86_64-qtwebkit
fi


## Prepare

# construct version string
BUILD="HUI-mingw-$(uname -m)-webkit-qt5"

# create target directory 
mkdir -p $BUILD
cd $BUILD

# TODO: qmake *.moc


## Build

# build with simple g++
g++ -shared -fPIC -o ./libHUI.dll ../hui_webview__webkit_qt5.cc -I.. `pkg-config --cflags --libs Qt5Widgets Qt5WebKitWidgets`


## Deploy

if [[ " $@ " =~ " --deploy " ]]; then

	# copy all non-system libraries (makes the build self-contained)
	ldd ./libHUI.dll | grep -vE /c/WINDOWS\|System32\|SYSTEM32 | awk '{print $3}' | xargs -I {} sh -c '[ -f {} ] && cp {} .'

	# add Qt5 stuff
	cp -r /C/msys64/mingw64/share/qt5/plugins/platforms .
	mkdir plugins
	cp -r /C/msys64/mingw64/share/qt5/plugins/imageformats ./plugins
	
fi


## Tests

if [[ " $@ " =~ " --tests " ]]; then

	# build and deploy (the previous ldd command can be omitted since it works recursively)
	#g++ -o <your_executable> <your_source> -L. -lHUI
	#ldd <your_executable> | grep -vE /c/WINDOWS\|System32\|SYSTEM32 | awk '{print $3}' | xargs -I {} sh -c '[ -f {} ] && cp {} .'

	# build tests
	g++ -o test_webview_js_api.exe ../tests/test_webview_js_api.cc -I.. -L. -lHUI
	
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
# 
# Deploy Qt5 Windows build (MSYS2 mingw environment): \
#  Put compiled binary to new directory. \
#  Add dlls that `ldd <your binary>` outputs. \
#  Copy there `C:\msys64\mingw64\share\qt5\plugins\platforms` directory. \
#  Create `plugins` directory and copy inside `C:\msys64\mingw64\share\qt5\plugins\imageformats`. \
#  In case you have built python module, the python interpreter have to be able to locate these files (this will likely mean adding python interpreter next to the module file).

# since kdewebkit package is not availible, it was replaced by its own dependencies (the used package archive was 'mingw-w64-x86_64-kdewebkit-qt5-5.114.0-1-any.pkg.tar.zst' from <https://mirror.jmu.edu/pub/msys2/mingw/mingw64/>)
# TODO: instaling required libs on debian derivates may be possible - try to find kdewebkit or its dependencies


cd ..