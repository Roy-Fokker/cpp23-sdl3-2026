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
- CMake 4.2x and lower require CMAKE_EXPERIMENTAL flag
- This flag is required to enable `import std;` and general modules functionality
