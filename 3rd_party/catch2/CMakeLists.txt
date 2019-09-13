cmake_minimum_required(VERSION 3.0)

project(catch2)

# Prepare "Catch2" library for other executables
add_library(Catch2 INTERFACE)
add_library(Catch2::Test ALIAS Catch2)
target_include_directories(Catch2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

#Catch2 needs log if on android
if (CMAKE_SYSTEM_NAME MATCHES "Android")
    target_link_libraries(Catch2 INTERFACE log)
endif ()