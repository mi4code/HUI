#!/bin/bash

## Dependencies

# install dependencies
if [[ " $@ " =~ " --dependencies " ]]; then
       if [[ -f /etc/debian_version ]]; then
               # Debian/Ubuntu/Raspbian
               sudo apt install -y g++ cmake make curl tar bzip2 libgtk-3-dev libnss3 libgbm1 libasound2 libxkbcommon0 libxcomposite1 libxdamage1 libxfixes3 libxrandr2 libatk1.0-0 libatk-bridge2.0-0 libcups2 libdrm2 libatspi2.0-0
       elif [[ -f /etc/arch-release ]]; then
               # Arch/Manjaro
               sudo pacman -S --noconfirm base-devel cmake make curl tar bzip2 gtk3 nss dbus libx11 libxcomposite libxdamage libxext libxfixes libxrandr libxcb libxkbcommon libdrm libexpat alsa-lib at-spi2-core
       else
               echo "unable to install dependencies - unsupported system"
       fi
fi


## Prepare

# cef version - keep in sync with build_win_cef.bat (pinned on purpose - this is the last cef we use that supports the single process mode we rely on)
CEF_VERSION="131.2.3+g023a05b+chromium-131.0.6778.70"

case "$(uname -m)" in
       x86_64)  CEF_PLATFORM="linux64";    CEF_PROJECT_ARCH="x86_64" ;;
       aarch64) CEF_PLATFORM="linuxarm64"; CEF_PROJECT_ARCH="arm64"  ;;
       *) echo "unsupported architecture: $(uname -m) - cef only ships linux64 and linuxarm64"; exit 1 ;;
esac

# construct version string
BUILD="HUI-linux-$(uname -m)-blink-cef"

# create target directory
mkdir -p $BUILD
cd $BUILD

# download from https://cef-builds.spotifycdn.com/index.html (linux64 / linuxarm64, minimal) and extract
# ~470 MB download and ~2.1 GB extracted (libcef.so alone is ~2 GB, it ships with debug info)
# skipped if already present so reruns dont re-download
if [ ! -d ./_build_cache_cef_binary ]; then
       curl -L -o cef.tar.bz2 "https://cef-builds.spotifycdn.com/cef_binary_${CEF_VERSION//+/%2B}_${CEF_PLATFORM}_minimal.tar.bz2"
       tar xjf cef.tar.bz2
       mv ./cef_binary_*_${CEF_PLATFORM}_minimal ./_build_cache_cef_binary
       rm -f ./cef.tar.bz2
fi

# add files to the build tree
cp ../cef_CMakeLists.txt ./CMakeLists.txt


## Build

# build directory for cmake
mkdir -p _build_cache_cef_build
cd _build_cache_cef_build

# build with cmake
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DPROJECT_ARCH=$CEF_PROJECT_ARCH ..
cmake --build . --config Release -j $(nproc)

# go back to $BUILD
cd ..

# delete cmake file
rm -f ./CMakeLists.txt


## Deploy

if [[ " $@ " =~ " --deploy " ]]; then
       # cef runtime (Release binaries + Resources, which brings locales/ along) next to libHUI.so, so the $ORIGIN rpath baked into libHUI.so finds libcef.so at runtime
       cp -f ./_build_cache_cef_binary/Release/* .
       cp -rf ./_build_cache_cef_binary/Resources/* .
       cp -f ./_build_cache_cef_build/bin/libHUI.so .
       cp -f ./_build_cache_cef_build/bin/cef_shim_linux.so .
else
       # just copy libHUI.so so it works the same as other scripts
       cp -f ./_build_cache_cef_build/bin/libHUI.so .
       cp -f ./_build_cache_cef_build/bin/cef_shim_linux.so .
fi

# delete builded files
# note: unlike build_win_cef.bat we keep ./_build_cache_cef_binary - it is the cache that
# saves re-downloading ~470 MB on the next run
rm -rf ./_build_cache_cef_build


## Tests
if [[ " $@ " =~ " --tests " ]]; then
       g++ -o test_webview_js_api ../tests/test_webview_js_api.cc -I../{c,c++} -L. -lHUI
fi


## Notes

# the deployed libcef.so ships with debug info and is ~2 GB; stripping the deployed copy is optional: strip ./libcef.so
# on linux libcef.so AND cef_shim_linux.so are both DT_NEEDEDs of libHUI.so - the shim is listed *after* libcef.so (its DT_SONAME is cef_shim_linux.so, so the DT_NEEDED records the plain name) and RUNPATH=$ORIGIN resolves both from libHUI.so's own directory. A DT_NEEDED is in the link map before any constructor runs, so this answers libcef.so's dlsym(RTLD_NEXT,"localtime"/"localtime_r") lookups at load time as well as at runtime; the previous dlopen() scheme only covered the runtime ones. Both are hard dependencies: if cef_shim_linux.so is missing next to libHUI.so the loader refuses to load libHUI.so at all with "cannot open shared object file" (loud, immediate) instead of libHUI.so printing a warning and continuing. So libcef.so + cef_shim_linux.so (+ locales/ + *.pak) must sit next to libHUI.so or be reachable via LD_LIBRARY_PATH - the host itself needs no cef link flag, it only ever links -lHUI


cd ..
