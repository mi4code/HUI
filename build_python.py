#!/usr/bin/env python3
## HUI build script, expects to be run from build directory

import os, platform, sys, sysconfig

# build dummy or use existing? -yes
# should result in only module file

system = platform.system()

if system == "Windows":
    if "MSYSTEM" in os.environ:  # echo $MSYSTEM => MINGW/...
        print("building on windows (mingw)...")
        print("installing dependencies...")
        os.system("pacman -S --noconfirm --needed mingw-w64-x86_64-pybind11")
        print("running build...")
        os.system('bash -c "g++ -O3 -shared -fPIC ../hui_python.cc -o HUI$(python3-config --extension-suffix) -I.. -L. -lHUI $(python3 -m pybind11 --includes) -lpython'+str(sys.version_info.major)+'.'+str(sys.version_info.minor)+'"')  # '-lpython3.12' needed to avoid 'undefined reference to'
        
    else:
        print("building on windows (msvc)...")
        print("installing dependencies...")
        os.system("pip install --no-input pybind11")
        print("running build...")
        includes = (" "+os.popen("python -m pybind11 --includes").read()[:-1]).split(" -I")[1:] # gets them in g++ style, remove newline, split them by -I, get rid of first blank => list of paths; then put them together
        includes = " ".join(['/I"'+i+'"' for i in includes])  # /I"C:\Program Files\Python312\Include" /I"C:\Users\%username%\AppData\Roaming\Python\Python312\site-packages\pybind11\include"
        libpath = sys.base_prefix  # C:\Program Files\Python312
        pyname = "python"+str(sys.version_info.major)+str(sys.version_info.minor)  # python312
        suffix = sysconfig.get_config_var('EXT_SUFFIX')  # .cp312-win_amd64.pyd
        os.system("call \"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat\"  &&  cl /LD /EHsc /O2 /Fe:HUI"+suffix+" ../hui_python.cc "+includes+" /I\"..\" /link /LIBPATH:\""+libpath+"\\libs"+"\" "+pyname+".lib libHUI.lib")
        
    print("done")


elif system == "Linux":

    print("building on linux...")
    
    if os.path.isfile("/etc/debian_version"):
        print("debian derivate, installing dependencies...")
        os.system("sudo apt install -y python3-pybind11 python3-dev")
        
    elif os.path.isfile("/etc/arch-release"):
        print("arch derivate, installing dependencies...")
        os.system("sudo pacman -S --noconfirm --needed pybind11")
        
    else:
        print("unknown linux distro, hope you have everything installed...")
        
    print("running build...")
    os.system("g++ -O3 -shared -fPIC ../hui_python.cc -o HUI$(python3-config --extension-suffix) -I.. -L. -lHUI $(python3 -m pybind11 --includes)")
    print("done")


else:

    print("unsupported os, sorry")


input()

"""
OLD NOTES:

##### GTK3
Build as Python module: \
 `` g++ -DHUI_BUILD_CPP -DHUI_BUILD_PYTHON -shared -fPIC -x c++ -O3 -o HUI$(python3-config --extension-suffix) ./li-hui/hui_new.hh $(python3 -m pybind11 --includes) `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0 gtk-layer-shell-0` ``

##### Qt5

Build Python module (for python in MSYS2 mingw64) under Windows in MSYS2 mingw: \
 `` g++ -DHUI_BUILD_CPP -DHUI_BUILD_PYTHON -shared -fPIC -x c++ -O3 -o HUI$(python3-config --extension-suffix) ./hui_new.hh $(python3 -m pybind11 --includes) `pkg-config --cflags --libs Qt5Widgets Qt5WebKitWidgets` -lpython3.11 `` \
 (note: replace  `-lpython3.11` with coresponding value from output of `python3-config --ldflags`)

Build Python module (for system python) under Windows in MSYS2 mingw: \
 Figure out python module suffix (try searching .pyd in python instalation folder). \
 In Windows CMD run: `python3 -m pybind11 --includes` to get pybind11 includes. \
 Find python instalation folder (should contain `python.exe` and `libpython*.dll`) using `python -c "import os, sys; print(os.path.dirname(sys.executable))"`. \
 Then fill these in this command and run it inside mingw64 (dont forget to translate paths to correct format): `` g++ -DHUI_BUILD_CPP -DHUI_BUILD_PYTHON -shared -fPIC -x c++ -O3 -o HUI<module suffix> ./hui_new.hh <pybind11 includes> `pkg-config --cflags --libs Qt5Widgets Qt5WebKitWidgets` <include pxthon dll from instalation folder> `` \
 (for example: `` g++ -DHUI_BUILD_CPP -DHUI_BUILD_PYTHON -shared -fPIC -x c++ -O3 -o HUI.cp312-win_amd64.pyd ./hui_new.hh "-I/c/Program Files/Python312/Include" -I/c/Users/mi4code/AppData/Roaming/Python/Python312/site-packages/pybind11/include `pkg-config --cflags --libs Qt5Widgets Qt5WebKitWidgets` "-L/c/Program Files/Python312" -lpython312 ``)




"""