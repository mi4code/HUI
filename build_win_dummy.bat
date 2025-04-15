@echo off
setlocal enabledelayedexpansion

:: put cmd args into string and add spaces around them
set "ARGS= %* "
:: argument check - replaces the flag (with spaces around) with an empty string and than compares the original and the repalced one
::if not "%ARGS%" == "%ARGS: --flag =%" echo "the flag is set"


REM Dependencies

if not "%ARGS%" == "%ARGS: --dependencies =%" (

	:: TODO: install Visual Studio Native Build Tools
	echo installing dependencies not supported yet

)


REM Prepare

:: build directory
set BUILD=HUI-win-%PROCESSOR_ARCHITECTURE%-dummy
mkdir %BUILD% 2>nul
cd %BUILD%
:: TODO: lowercase architecture or/and standardise architecture strings 


REM Build

:: launch vc cmdline 
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

:: build
cl /LD /EHsc /D"and=&" /Fe:libHUI.dll ..\hui_webview__dummy.cc

:: create definiton file (dumpbin output edited to be "EXPORTS \n HUI_<list of all methods> \n ...") - its easier to have the file prepared and just copy it
::dumpbin.exe /EXPORTS HUI.dll > HUI.def
::notepad HUI.def
copy ..\cef_HUI.def .\libHUI.def
:: TODO: rename the file as its not cef specific

:: generate *.lib
lib /def:libHUI.def /machine:x64 /out:libHUI.lib


REM Deploy
if not "%ARGS%" == "%ARGS: --deploy =%" (
	echo deploying not needed here
)


REM Tests

if not "%ARGS%" == "%ARGS: --tests =%" (
	
	:: build tests
	cl /EHsc /Fe:test_webview_js_api.exe ../tests/test_webview_js_api.cc  /I.. /link /LIBPATH:. libHUI.lib
	
)


REM Notes


cd ..
endlocal