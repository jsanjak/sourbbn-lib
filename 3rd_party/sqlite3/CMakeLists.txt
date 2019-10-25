cmake_minimum_required(VERSION 3.0)

project(sqlite3)

add_library(sqlite3 sqlite3/sqlite3.c sqlite3/sqlite3.h)

#Catch2 needs log if on android
if (CMAKE_SYSTEM_NAME MATCHES "Android")
    target_link_libraries(sqlite3 INTERFACE log)
endif ()