cmake_minimum_required(VERSION 4.2 FATAL_ERROR)

# Std Module Config
# Set the experimental flag, needs to be changed per CMake version being used.
# https://github.com/Kitware/CMake/blob/master/Help/dev/experimental.rst#c-import-std-support
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
 	"d0edc3af-4c50-42ea-a356-e2862fe7a444" # updated value for 4.0.3 ... 4.1.x
)

# tell CMake we want to use 'import std'
# will get enabled for all targets declared after this
set(CMAKE_CXX_MODULE_STD 1)