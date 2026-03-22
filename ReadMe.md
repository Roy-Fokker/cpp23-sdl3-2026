# C++23 & SDL3 GPU Project

Redo of the project template using SDL3 GPU with C++23.

Using what was learned from old template and new knowledge gained since then.


## Build Dependencies
- Ninja Build
- CMake 4.+
- Clang 21+
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
- ~~absolute path to `compile_commands.json` file's directory is required.~~  
  Apparently, it is possible to use relative path from `.clangd` is possible, but...
  - this file not very portable, must be configured for each environment/build machine


## SDL3 pecularities
- requires installation of additional dependencies on linux,  
  on `PikaOS` using `Pikman package manager`
  ```bash
  pikman i libxcursor-dev libgbm-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev \
           libwayland-dev libxcursor-dev libxrandr-dev libxkbcommon-dev libdrm-dev      \
           libudev-dev libdbus-1-dev libibus-1.0-dev libxi-dev                          \
           libxss-dev libxtst-dev                                                       \
           libpipewire-0.3-dev libdecor-0-dev
  ```
- On Wayland (linux), basic gpu device requires calling all the supporting frame/swapchain stuff. Else it will error out and no window will be shown. 

## GCC issues
While there is a preset for GCC in the `CmakePresets.json`, project doesn't build, as GCC is throwing errors on compile. Suspect some flags need to be enabled for GCC to compile modules.