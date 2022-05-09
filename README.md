# McEngine
A small cross-platform engine/framework/sandbox for drawing stuff to a window with GPU acceleration, handling various input devices, outputting sound, and networking. Mostly aimed at game development, although it does not (and will never) have a visual editor like Unity, Unreal or CryEngine, so everything is written in C++. It's more or less my little coding playground, a place to experiment and improve over time, and may be useful to other people as well. As it contains code from since I started learning programming (2009) to today, quality varies a bit across not-yet-refactored-and-cleaned-up files.

**Design Philosophy:**

- Very thin wrappers around OS and graphics APIs (e.g. at most one level of indirection or vtable lookup)
- Lean dependencies (e.g. a C++ compiler/toolchain is the only requirement to get started, all dependencies are wrapped and replaceable)
- No preempting of any specific development or architectural game code style (e.g. there will never be a generic "Entity" class of any kind)
- Rapid prototyping (e.g. loading and drawing an image is two lines of code, moving the image with a key three more)

**About the structure:**

Inside the McEngine project folder are three subfolders, ```build``` &amp; ```libraries``` &amp; ```src```.

1. ```/build/``` contains the working directory of the engine (where the executable _would_ go into)

2. ```/libraries/``` contains all dependencies (includes, dlls, libs, sos)

3. ```/src/``` contains the source code
   1. ```/src/App/``` contains the code for applications using the engine
   2. ```/src/Engine/``` contains the core
      1. ```/src/Engine/Input/``` contains input devices
      2. ```/src/Engine/Main/``` contains the main entry points
      3. ```/src/Engine/Platform/``` contains all platform specific code which is not in Main
      4. ```/src/Engine/Renderer/``` contains renderer specific code
   3. ```/src/GUI/``` contains very primitive UI elements, mostly for debugging (such as the console)
   4. ```/src/Util/``` contains helper functions, and small libraries which are header-only or contained in one file

- Every supported platform must have a ```main_<OS_NAME>.cpp``` file in ```/src/Engine/Main/``` containing the main entry point.
- Other platform specific code which is not part of the main file goes into ```/src/Engine/Platform/```.
- Hiding platform specific code is done by using trivial ```#ifdefs```, meaning that the exact same codebase can instantly be compiled without any changes on different platforms.
- I don't like relative includes, therefore _every single (sub)directory_ which is needed is added as an include path to the compiler.
- Separate applications using the engine go into ```/src/App/```. The ```FrameworkTest``` app is hardcoded by default.
- Every application must be started by including its header in ```/src/Engine/Engine.cpp``` as well as instantiating it in ```Engine::loadApp()``` atm.
- Not all libraries are necessary for every project, you can enable or disable certain libraries completely by using defines in ```/src/Engine/Main/EngineFeatures.h```, such as OpenCL, OpenVR, XInput, BASS or ENet. Everything will always compile and run, even if you use features which are not enabled.

**Projects using the engine:**

[https://github.com/McKay42/McOsu](https://github.com/McKay42/McOsu)

[https://github.com/McKay42/McOsu-NX](https://github.com/McKay42/McOsu-NX)

[https://github.com/McKay42/mph-model-viewer](https://github.com/McKay42/mph-model-viewer)

[Yesterday (university course project, YouTube link)](https://www.youtube.com/watch?v=RbuP1dNG304)

## Building

### Scripts
Included in the repository are platform specific zero dependency build scripts, so all you need is a C++ compiler/toolchain.  
For example, the Windows `build.bat` script only depends on having `g++.exe` and `gcc.exe` available in your `PATH`.

There are `build.bat` and `build.sh` as well as `run.bat` and `run.sh` for their respective platforms.

- The build scripts are meant for one-off and automated builds
- The build scripts will always do full builds (and **not** incremental builds)
- The build scripts will always create release builds (and **not** debug builds)
- The build scripts are also meant for documenting compiler flags/paths/libs/etc.

There are also `build_unity.bat` and `build_unity.sh` for their respective platforms.

- The unity build scripts are meant for development without an IDE
- The unity build scripts support partial incremental builds if run with the `incremental` command line argument
- The unity build scripts incremental builds will still always recompile all C++ files as one big unity.cpp file (but not all C files)
- The unity build scripts are expected to be manually customized for debug builds

### Eclipse
This is a preconfigured Eclipse CDT (C/C++) project, meaning that in the best case you only have to git clone the repository into your workspace folder and File > Import > "Existing Projects into Workspace". You can get it here:

[https://www.eclipse.org/downloads/eclipse-packages/](https://www.eclipse.org/downloads/eclipse-packages/)

- In Eclipse, make sure you select the correct build configuration. All libraries, paths, includes and flags are already set.
On Windows, the build configuration to use would be either ```Windows Release``` or ```Windows Debug```.

- Eclipse may complain that the ```Toolchain "MinGW GCC" is not detected```, just ignore it. As long as g++ and all the other tools are in your `PATH`, everything should work fine.

- Make sure you create a correct ```Run Configuration```. The working directory must be ```${workspace_loc:McEngine/build}```, and the C/C++ Application path must be ```Windows Release/McEngine.exe``` if the ```Windows Release``` build config was used for building, or ```Windows Debug/McEngine.exe``` if the ```Windows Debug``` build config was used for building etc.

If you don't want to use Eclipse then either use the included build scripts mentioned before or create your own makefiles.  

### Windows
The Windows build needs Mingw-W64 (**i686** with **Win32** threads and **seh** exceptions), you can get it here:

[https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/](https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/)

- Pick the newest version available, currently 8.1.0.  
Note that newer versions are available at [https://winlibs.com/#download-release](https://winlibs.com/#download-release), but have not been tested nor validated by me.

- Extract e.g. ```x86_64-8.1.0-release-win32-seh-rt_v6-rev0.7z``` and copy the contained ```mingw64``` folder to the root of a drive, resulting in e.g. ```C:\mingw64```.

- Make sure you add the e.g. ```C:\mingw64\bin``` directory to your `PATH` (environmental variable). To test if everything works, try to enter the following command in a cmd window: `g++ --version`.

- Alternatively and/or additionally, you can also configure the environment inside the Eclipse project settings (C/C++ Build > Environment), by setting `MINGW_HOME` (e.g. `C:\mingw64`) and `PATH` (e.g. `C:\mingw64\bin`) accordingly.

If you don't want to use Eclipse then just run the included `build.bat` and `run.bat`.

**Microsoft Visual Studio and their compilers (MSVC) are not supported and never will be.**


### Linux
The Linux build needs the following packages (Ubuntu example here):
- sudo apt-get install build-essential
- sudo apt-get install mesa-common-dev
- sudo apt-get install libglu1-mesa-dev
- sudo apt-get install libx11-dev
- sudo apt-get install xorg-dev

If you don't want to use Eclipse then just run the included `build.sh` and `run.sh`.


### macOS
The macOS build needs the following tweaks:
- In Eclipse > Right click on the imported project > Properties > C/C++ General > File Types
    1. Select "Use Project Settings"
    2. Click on "New..."
        - Pattern: ```*.h```
        - Type: ```C++ Header File```
        - Click on "OK"
    3. Click on "New..."
        - Pattern: ```*.cpp```
        - Type: ```C++ Source File```
        - Click on "OK"
    4. Click on "New..."
        - Pattern: ```*.mm```
        - Type: ```C++ Source File```
        - Click on "OK"
    5. Click on "OK"

Depending on which libraries you want to use, a few extra steps are required:  
(Note that both BASS + BASSFX and SDL2 are required by default, even if SDL2 is not enabled.)

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
       - TODO (or use the included ```libbass.dylib``` + ```libbass_fx.dylib```)
   2. SDL2
       - Make sure you have a compiled build
       - Copy ```SDL2.framework/Versions/A/SDL2``` to a temporary location (or use the included ```SDL2.dylib```)
           - Rename ```SDL2``` to ```SDL2.dylib```
           - Open a Terminal, cd into the temporary location
           - ```install_name_tool -id "@loader_path/SDL2.dylib" SDL2.dylib```
           - Validate the change with ```otool -l SDL2.dylib```
       - Copy ```SDL2.dylib``` into the ```/build/``` directory
	   - Open a Terminal, cd into the ```/build/``` directory
	   - Validate the file signature with ```codesign -vvvv SDL2.dylib```
	   - If the file signature of ```SDL2.dylib``` is invalid, then ```codesign -f -s - SDL2.dylib``` (note the dash)
       - Open a Terminal, cd into the ```/MacOS Release/``` directory (if this is a macOS release of course)
       - ```install_name_tool -change @rpath/SDL2.framework/Versions/A/SDL2 @loader_path/SDL2.dylib McEngine```
       - Validate the change with ```otool -l McEngine```     
