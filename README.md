# McEngine
Currently empty, only using it for issue &amp; todo tracking

##Eclipse
This is a preconfigured Eclipse CDT project, meaning that in the best case you only have to copy the McEngine folder (which contains src, libraries, build and the project files) to your workspace and import it in Eclipse as an existing project.

[https://www.eclipse.org/downloads/eclipse-packages/](https://www.eclipse.org/downloads/eclipse-packages/)

- In Eclipse, make sure you select the correct build configuration. All libraries, paths, includes, flags are already set.
For Windows, the build configuration to use would be either ```Windows Release``` or ```Windows Debug```.

- Eclipse may complain that ```Toolchain "MinGW GCC" is not detected```, just ignore it. As long as g++ and all the other tools are in your PATH, everything should work fine.

- Make sure you create a correct ```Run Configuration```. The working directory must be ```${workspace_loc:McEngine\build}```, and the C/C++ Application path must be ```Windows Release/McEngine.exe``` (if this is a Windows release of course).

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
