cmake_minimum_required(VERSION 3.0)

project(sqlite3)

add_library(sqlite3 sqlite3/sqlite3.c sqlite3/sqlite3.h)

set_target_properties(sqlite3 PROPERTIES PUBLIC_HEADER sqlite3/sqlite3.h)

install(TARGETS sqlite3
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include)



#Catch2 needs log if on android
if (CMAKE_SYSTEM_NAME MATCHES "Android")
    target_link_libraries(sqlite3 INTERFACE log)
endif ()