D3PP
----
The [D3classic](https://github.com/umby24/D3classic) server, ported to C++.

Porting it for better maintainability and for fun.

## Compiling on Windows

1. Install VSCode, msys2 (x64)
2. Install the winpthread library (pacman -S mingw-w64-clang-x86_64-libwinpthread-git)
3. Install the dev zlib library (pacman -S zlib-devel)
4. Install the CMake Library (pacman -S mingw-w64-x86_64-cmake)
5. Add msys mingw64 to your PATH.
6. Open VSCode, install the CMake Tools and C++ Plugins.
7. Open the command pallet (ctrl+shift-p), go to preferences: open settings.json, and enter the following (replacing for your path if different);
```
    "cmake.cmakePath": "C:\\msys64\\mingw64\\bin\\cmake.exe",
    "cmake.mingwSearchDirs": ["C:\\msys64\\mingw64\\bin"],
    "cmake.generator": "MinGW Makefiles",
```
8. Restart VS code
9.  Clone the repo and open it
10. Configure your C++ kits to GCC/Mingw32 (may require a manual scan)
11. Press the build button!