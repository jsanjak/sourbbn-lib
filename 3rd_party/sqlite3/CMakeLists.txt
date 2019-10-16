cmake_minimum_required(VERSION 3.0)

project(sqlite3)

# Prepare SQLite3 library for other executables
add_library(sqlite3 INTERFACE)
target_include_directories(sqlite3 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

#SQLite3 needs log if on android
if (CMAKE_SYSTEM_NAME MATCHES "Android")
    target_link_libraries(sqlite3 INTERFACE log)
endif ()