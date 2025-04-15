@echo off
setlocal enabledelayedexpansion

:: put cmd args into string and add spaces around them
set "ARGS= %* "
:: argument check - replaces the flag (with spaces around) with an empty string and than compares the original and the repalced one
::if not "%ARGS%" == "%ARGS: --flag =%" echo "the flag is set"


REM Dependencies

if not "%ARGS%" == "%ARGS: --dependencies =%" (

	:: TODO: install Visual Studio Native Build Tools + 7zip in path
	echo installing dependencies not supported yet

)


REM Prepare

:: build directory
set BUILD=HUI-win-%PROCESSOR_ARCHITECTURE%-blink-cef
mkdir %BUILD% 2>nul
cd %BUILD%
:: TODO: lowercase architecture or/and standardise architecture strings 

:: download from https://cef-builds.spotifycdn.com/index.html#windows64 (recommended minimal from 16.11.2024 - https://cef-builds.spotifycdn.com/cef_binary_131.2.3%2Bg023a05b%2Bchromium-131.0.6778.70_windows64_minimal.tar.bz2) and extract
curl -L -o cef.tar.bz2 "https://cef-builds.spotifycdn.com/cef_binary_131.2.3%%2Bg023a05b%%2Bchromium-131.0.6778.70_windows64_minimal.tar.bz2"
7z x cef.tar.bz2 -so | 7z x -si -ttar -o.
ren ".\cef_binary_131.2.3+g023a05b+chromium-131.0.6778.70_windows64_minimal" "_build_cache_cef_binary"
del /q .\cef.tar.bz2

:: add files to the build tree
copy ..\cef_CMakeLists.txt .\CMakeLists.txt


REM Build

:: build directory for cmake
mkdir _build_cache_cef_build
cd _build_cache_cef_build

:: launch vc cmdline 
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

:: build with cmake
cmake -DCMAKE_BUILD_TYPE=Release .. 
cmake --build . --config Release

:: go back to %BUILD%
cd ..

:: delete cmake file
del /q .\CMakeLists.txt


REM Deploy

if not "%ARGS%" == "%ARGS: --deploy =%" (

	:: TODO: deploy info
	copy .\_build_cache_cef_binary\Release\* .
	copy .\_build_cache_cef_binary\Resources\* .
	copy .\_build_cache_cef_build\bin\Release\* .
	mkdir .\locales
	copy .\_build_cache_cef_binary\Resources\locales\* .\locales

	:: delete downloaded files
	rd /s /q .\_build_cache_cef_binary
	
) else (
	
	:: just copy libHUI.dll so it works the same as other scripts
	copy .\_build_cache_cef_build\bin\Release\libHUI.dll .

)

:: create definiton file (dumpbin output edited to be "EXPORTS \n HUI_<list of all methods> \n ...") - its easier to have the file prepared and just copy it
copy ..\cef_HUI.def .\libHUI.def

:: create the lib file
lib /def:libHUI.def /machine:x64 /out:libHUI.lib

:: delete builded files
rd /s /q .\_build_cache_cef_build


REM Tests

if not "%ARGS%" == "%ARGS: --tests =%" (
	
	:: build tests
	cl /EHsc /Fe:test_webview_js_api.exe ../tests/test_webview_js_api.cc  /I.. /link /LIBPATH:. libHUI.lib
	
)

:: cl (recomended):
::cl /EHsc /Fe:<binary>.exe ../<source> /I.. /link /LIBPATH:. HUI.lib

:: g++ (not recommended, broken; doesnt need def.lib; mingw):
::g++ -o <binary>.exe ../<source> -L. -lHUI -I..


REM Notes


cd ..
endlocal
