# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

project(sourbbn-lib)

#Use C++11 standard with -fPIC
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# add the CMakeFile that defines catch2--used for unit testing
add_subdirectory(3rd_party/catch2)
add_subdirectory(3rd_party/sqlite3)

include_directories(3rd_party/sqlite3/sqlite3)

#Adds the library with no build products
file(GLOB sour_SRC "src/*cpp")
add_library(sourbbn-lib SHARED ${sour_SRC})

#Include the header files not tests or examples
target_include_directories(sourbbn-lib PUBLIC include)
target_link_libraries(sourbbn-lib sqlite3)
##########################################################
# Unit tests
set( CMAKE_CXX_FLAGS " -pthread " )
enable_testing()

add_executable(unit_tests test/unit_tests.cpp)
add_executable(scratch_test test/scratch_test.cpp)

target_link_libraries(unit_tests
    sourbbn-lib
    Catch2::Test
    sqlite3
    ${CMAKE_DL_LIBS}
)

target_link_libraries(scratch_test
    sourbbn-lib
    sqlite3
    ${CMAKE_DL_LIBS}
)


#add_test(test_all unit_tests)

