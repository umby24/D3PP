![D3PP](./d3pp_logo.png)
----

[![discord](https://img.shields.io/badge/Discord-blue?style=for-the-badge)](https://discord.gg/CcnAaZpy)

The [D3classic](https://github.com/umby24/D3classic) server, ported to C++.

Porting it for better maintainability and for fun.

## Roadmap
 - ~~Port the primary server functionality (See issue #1)~~
 - ~~First pass refactor~~
 - Bring up to date with latest CPE (In Progress)
 - Second pass refactor
 - Bring in lesser used features if requested

## Not slated to port

Currently, there are a few parts of the existing D3 server that I'm not planning to port. Those are:
 - Map Overview Images
 - Multi-Language support
 - Building text from a font .png file
 - External C/C++ Plugins

If you would like those reach out to let me know, and of course Pull Requests are welcome if you would like to contribute.

## Compiling on Windows

1. Install VSCode, msys2 (x64)
2. Install the winpthread library (pacman -S mingw-w64-clang-x86_64-libwinpthread-git)
3. Install the dev zlib library (pacman -S zlib-devel)
4. Install the CMake Library (pacman -S mingw-w64-x86_64-cmake)
5. Install the Lua Library (pacman -S mingw-w64-x86_64-lua)
6. Add msys mingw64 to your PATH.
7. Open VSCode, install the CMake Tools and C++ Plugins.
8. Open the command pallet (ctrl+shift-p), go to preferences: open settings.json, and enter the following (replacing for your path if different);
```
    "cmake.cmakePath": "C:\\msys64\\mingw64\\bin\\cmake.exe",
    "cmake.mingwSearchDirs": ["C:\\msys64\\mingw64\\bin"],
    "cmake.generator": "MinGW Makefiles",
```
9. Restart VS code
10.  Clone the repo and open it
11. Configure your C++ kits to GCC/Mingw32 (may require a manual scan)
12. Press the build button!

## Compiling on Windows (MSVC)

1. Make sure you have Visual Studio with C++ Make tools installed
2. Install vcpkg (instructions here)
3. install the requisite libraries (`vcpkg install Lua` , `vcpkg install SQLite3`, `vcpkg install zlib`)
3a. If you are using 64 bit builds, add :x64-windows to the end (`vcpkg install SQLite3:x64-windows`)
4. Clone the repo and open visual studio. Use the open folder option to open the folder.
5. CMake should auto-detect and autoconfigure, and you should be good to go!

## Compling on Linux
These instructions used on Fedora

# install your distros build-essential files
# Install lua
# clone the repo and open it up in terminal
# mkdir build && cd build
# cmake ../
# make