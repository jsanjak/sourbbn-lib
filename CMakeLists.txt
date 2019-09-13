# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

project(sourbbn-lib)

#Use C++11 standard
set(CMAKE_CXX_STANDARD 11)

# add the CMakeFile that defines catch2--used for unit testing
add_subdirectory(3rd_party/catch2)

#Adds the library with no build products
add_library(sourbbn-lib SHARED src/sourbbn.cpp)

#Include the header files not tests or examples
target_include_directories(sourbbn-lib PUBLIC include)


##########################################################
# Unit tests

enable_testing()

add_executable(unit_tests test/unit_tests.cpp)

target_link_libraries(unit_tests
    sourbbn-lib
    Catch2::Test
)

add_test(test_all unit_tests)

