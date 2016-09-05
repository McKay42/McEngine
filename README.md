# McEngine
A small cross-platform engine/framework/sandbox for drawing stuff to a window with GPU acceleration, handling various input devices, outputting sound, and networking. Mostly aimed at game development, although it does not have a baby editor like Unreal or CryEngine, (everything must be written in C++ atm). It's more or less my little coding playground, and may be useful to other people as well.

**About the structure:**

Inside the McEngine project folder are three subfolders, ```build``` &amp; ```libraries``` &amp; ```src```.

1. ```/build/``` contains the working directory of the engine (where the executable _would_ go into)

2. ```/libraries/``` contains all dependencies (includes, dlls, libs, sos)

3. ```/src/``` contains the source code
  3. ```/src/App/``` contains the code for applications using the engine
  4. ```/src/Engine/``` contains the core
    5. ```/src/Engine/Input/``` contains input devices
    6. ```/src/Engine/Main/``` contains the main entry points
    7. ```/src/Engine/Platform/``` contains all platform specific code which is not in main
    8. ```/src/Engine/Renderer/``` contains renderer specific code
  5. ```/src/GUI/``` contains very primitive UI elements, mostly for debugging (such as the console)
  6. ```/src/Util/``` contains helper functions, and small libraries which are contained in one file

- Every supported platform must have a ```main_<OS_NAME>.cpp``` file in ```/src/Engine/Main/``` containing the main entry point.
- Other platform specific code which is not part of the main file goes into ```/src/Engine/Platform/```.
- Hiding platform specific code is done by using trivial ```#ifdefs```, meaning that the exact same codebase can instantly be compiled without any changes on different platforms.
- I don't like relative includes, therefore _every single (sub)directory_ which is needed is added as an include path to the compiler.
- Separate applications using the engine go into ```/src/App/```. The ```FrameworkTest``` app is hardcoded by default.
- Every application must be started by including its header in ```/src/Engine/Engine.cpp``` as well as instantiating it in ```Engine::loadApp()``` atm.

##Eclipse
This is a preconfigured Eclipse CDT project, meaning that in the best case you only have to copy the McEngine folder (which contains src, libraries, build and the project files) to your workspace and import it in Eclipse as an existing project.

[https://www.eclipse.org/downloads/eclipse-packages/](https://www.eclipse.org/downloads/eclipse-packages/)

- In Eclipse, make sure you select the correct build configuration. All libraries, paths, includes, flags are already set.
For Windows, the build configuration to use would be either ```Windows Release``` or ```Windows Debug```.

- Eclipse may complain that the ```Toolchain "MinGW GCC" is not detected```, just ignore it. As long as g++ and all the other tools are in your PATH, everything should work fine.

- Make sure you create a correct ```Run Configuration```. The working directory must be ```${workspace_loc:McEngine/build}```, and the C/C++ Application path must be ```Windows Release/McEngine.exe``` (if this is a Windows release of course).

If you don't want to use eclipse you'll have to create your own makefile. Microsoft Visual Studio and their compilers are not supported.

##Windows
The Windows build needs mingw-w64 (i686 with Win32 threads), you can get it here:

[https://mingw-w64.org/doku.php/](https://mingw-w64.org/doku.php/)

[https://sourceforge.net/projects/mingw-w64/](https://sourceforge.net/projects/mingw-w64/)

Make sure you add the mingw directory to your PATH after the installation is finished.


##Linux
The Linux build needs the following packages:
- sudo apt-get install mesa-common-dev
- sudo apt-get install libglu1-mesa-dev
- sudo apt-get install libx11-dev
- sudo apt-get install xorg-dev
