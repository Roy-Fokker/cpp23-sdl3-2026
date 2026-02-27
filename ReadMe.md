# C++23 & SDL3 GPU Project

Redo of the project template using SDL3 GPU with C++23.

Using what was learned from old template and new knowledge gained since then.


## Build Dependencies
- Ninja Build
- CMake 4.+
- GCC 15 / Clang 21
- ClangD
- Clang-Format
- Clang-Tidy

## Library Dependencies
- SDL v3.4.x
- GLM
- FMT

## CMake pecularities
- Package management is via CPM.cmake
- CMake 4.3x and lower require CMAKE_EXPERIMENTAL flag
- This flag is required to enable `import std;` and general modules functionality

## ClangD pecularities
- absolute path to `compile_commands.json` file's directory is required.
  - makes this file not very portable, must be configured for each environment/build machine


## SDL3 pecularities
- requires installation of additional dependencies on linux
  ```bash
  libwayland-dev libpipewire-0.3-dev libdecor-0-dev libxcursor-dev \
  libxrandr-dev libxkbcommon-dev libdrm-dev libgbm-dev libgl1-mesa-dev \
  libgles2-mesa-dev libegl1-mesa-dev libdbus-1-dev libibus-1.0-dev libudev-dev \
  libxi-dev libxss-dev libxtst-dev

  ```