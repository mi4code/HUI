# Start of MSYS2 shell script


## Dependencies

if [[ " $@ " =~ " --dependencies " ]]; then
	# no dependencies (except build tools)
	pacman -S --needed base-devel gcc
fi


## Prepare

# construct version string
BUILD="HUI-mingw-$(uname -m)-dummy"

# create target directory 
mkdir -p $BUILD
cd $BUILD


## Build

# build with simple g++
g++ -shared -fPIC -o ./libHUI.dll ../hui_webview__dummy.cc -I..


## Deploy

if [[ " $@ " =~ " --deploy " ]]; then
	# copy all non-system libraries (makes the build self-contained)
	ldd ./libHUI.dll | grep -vE /c/WINDOWS\|System32\|SYSTEM32 | awk '{print $3}' | xargs -I {} sh -c '[ -f {} ] && cp {} .'
fi


## Tests

if [[ " $@ " =~ " --tests " ]]; then

	# build and deploy (the first ldd command can be omitted since it works recursively)
	#g++ -o <your_executable> <your_source> -L. -lHUI
	#ldd <your_executable> | grep -vE /c/WINDOWS\|System32\|SYSTEM32 | awk '{print $3}' | xargs -I {} sh -c '[ -f {} ] && cp {} .'

	# build tests
	g++ -o test_webview_js_api.exe ../tests/test_webview_js_api.cc -I.. -L. -lHUI
	
fi


## Notes


cd ..