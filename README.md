# McEngine
A small cross-platform engine/framework/sandbox for drawing stuff to a window with GPU acceleration, handling various input devices, outputting sound, and networking. Mostly aimed at game development, although it does not have a baby editor like Unreal or CryEngine, (everything must be written in C++ atm). It's more or less my little coding playground, and may be useful to other people as well.

**About the structure:**

Inside the McEngine project folder are three subfolders, ```build``` &amp; ```libraries``` &amp; ```src```.

1. ```/build/``` contains the working directory of the engine (where the executable _would_ go into)

2. ```/libraries/``` contains all dependencies (includes, dlls, libs, sos)

3. ```/src/``` contains the source code
   1. ```/src/App/``` contains the code for applications using the engine
   2. ```/src/Engine/``` contains the core
      1. ```/src/Engine/Input/``` contains input devices
      2. ```/src/Engine/Main/``` contains the main entry points (and icons for the executable)
      3. ```/src/Engine/Platform/``` contains all platform specific code which is not in main
      4. ```/src/Engine/Renderer/``` contains renderer specific code
   3. ```/src/GUI/``` contains very primitive UI elements, mostly for debugging (such as the console)
   4. ```/src/Util/``` contains helper functions, and small libraries which are contained in one file

- Every supported platform must have a ```main_<OS_NAME>.cpp``` file in ```/src/Engine/Main/``` containing the main entry point.
- Other platform specific code which is not part of the main file goes into ```/src/Engine/Platform/```.
- Hiding platform specific code is done by using trivial ```#ifdefs```, meaning that the exact same codebase can instantly be compiled without any changes on different platforms.
- I don't like relative includes, therefore _every single (sub)directory_ which is needed is added as an include path to the compiler.
- Separate applications using the engine go into ```/src/App/```. The ```FrameworkTest``` app is hardcoded by default.
- Every application must be started by including its header in ```/src/Engine/Engine.cpp``` as well as instantiating it in ```Engine::loadApp()``` atm.
- Not all libraries are necessary for every project, you can enable or disable certain libraries completely by using defines in ```/src/Engine/Main/EngineFeatures.h```, such as OpenCL, OpenVR, XInput, BASS or ENet. Everything will always compile and run, even if you use features which are not enabled.

**Projects using the engine:**

[https://github.com/McKay42/McOsu](https://github.com/McKay42/McOsu)

[https://github.com/McKay42/mph-model-viewer](https://github.com/McKay42/mph-model-viewer)

[Yesterday (university course project, YouTube link)](https://www.youtube.com/watch?v=RbuP1dNG304)

## Eclipse
This is a preconfigured Eclipse CDT (C/C++) project, meaning that in the best case you only have to copy the McEngine folder (which contains src, libraries, build and the project files) to your workspace and import it in Eclipse as an existing project.

[https://www.eclipse.org/downloads/eclipse-packages/](https://www.eclipse.org/downloads/eclipse-packages/)

- In Eclipse, make sure you select the correct build configuration. All libraries, paths, includes, flags are already set.
For Windows, the build configuration to use would be either ```Windows Release``` or ```Windows Debug```.

- Eclipse may complain that the ```Toolchain "MinGW GCC" is not detected```, just ignore it. As long as g++ and all the other tools are in your PATH, everything should work fine.

- Make sure you create a correct ```Run Configuration```. The working directory must be ```${workspace_loc:McEngine/build}```, and the C/C++ Application path must be ```Windows Release/McEngine.exe``` (if this is a Windows release of course).

If you don't want to use eclipse you'll have to create your own makefile. Microsoft Visual Studio and their compilers are not supported.

## Windows
The Windows build needs mingw-w64 (i686 with Win32 threads), you can get it here:

Project: [https://mingw-w64.org/doku.php/](https://mingw-w64.org/doku.php/)

Installer: [https://sourceforge.net/projects/mingw-w64/](https://sourceforge.net/projects/mingw-w64/)

__Please use the installer__, since it just works™. The pre-built toolchains on the project page for mingw-w64 are apparently outdated and don't contain all the necessary files for a Windows build, while the installer supports up to g++ 6.3.0.
Make sure you add the mingw directory to your PATH after the installation is finished. To test if everything works, try to enter the following command in a cmd window: `g++ --version`.


## Linux
The Linux build needs the following packages:
- sudo apt-get install mesa-common-dev
- sudo apt-get install libglu1-mesa-dev
- sudo apt-get install libx11-dev
- sudo apt-get install xorg-dev


## macOS
The macOS build needs the following tweaks:
- In Eclipse > Right click on the imported project > Properties > C/C++ General > File Types
    1. Select "Use Project Settings"
    2. Click on "New..."
        - Pattern: *.h
        - Type: C++ Header File
        - Click on "OK"
    3. Click on "New..."
        - Pattern: *.cpp
        - Type: C++ Source File
        - Click on "OK"
    4. Click on "New..."
        - Pattern: *.mm
        - Type: C++ Source File
        - Click on "OK"
    5. Click on "OK"

Depending on which libraries you want to use, a few extra steps are required:  
(Note that BASS + BASSFX is required/enabled by default, but SDL2 is not.)

- Libraries:
    1. BASS + BASSFX
        - Make sure you have a compiled build
        - Copy ```/build/libbass.dylib``` into ```/MacOS Release/``` (if this is a macOS release of course).
        - Copy ```/build/libbass_fx.dylib``` into ```/MacOS Release/``` (if this is a macOS release of course).
    2. SDL2
        - Either use the included dmg or download the Development Libraries here: https://www.libsdl.org/download-2.0.php
        - Mount the dmg
        - Copy ```SDL2.framework``` into ```/Library/Frameworks/```

Copying the libraries into the release directory is only necessary if you want to start the executable from within Eclipse, because on macOS the run configuration working directory does not work for libraries (i.e. the executable incorrectly looks for libraries inside the release directory, instead of inside the ```/build/``` directory, even if a custom working directory is specified).

The next section is optional, and explains how to create a standalone build without any dependencies, similar to how dlls can be put next to the executable on Windows (without requiring the user to install libraries system-wide).

- Standalone (Optional):
   1. BASS + BASSFX standalone
       - TODO (or use the included libbass.dylib + libbass_fx.dylib)
   2. SDL2
       - Make sure you have a compiled build
       - Copy ```SDL2.framework/Versions/A/SDL2``` to a temporary location (or use the included ```SDL2.dylib```)
           - Rename ```SDL2``` to ```SDL2.dylib```
           - Open a Terminal, cd into the temporary location
           - ```install_name_tool -id "@loader_path/SDL2.dylib" SDL2.dylib```
           - Validate the change with ```otool -l SDL2.dylib```
       - Copy SDL2.dylib into the ```/build/``` directory
       - Open a Terminal, cd into the ```/MacOS Release/``` directory (if this is a macOS release of course)
       - ```install_name_tool -change @rpath/SDL2.framework/Versions/A/SDL2 @loader_path/SDL2.dylib McEngine```
       - Validate the change with ```otool -l McEngine```     
