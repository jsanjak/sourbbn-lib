# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

project(sourbbn-lib)

#Adds the library with no build products
add_library(sourbbn-lib INTERFACE)

#Include the header files not tests or examples
target_include_directories(sourbbn-lib INTERFACE include/)

