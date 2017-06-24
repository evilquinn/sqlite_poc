
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(sqlite3 sqlite3.c)

add_executable(sqlite3_cli shell.c
                           sqlite3.c)

target_link_libraries(sqlite3 pthread
                              dl)

target_link_libraries(sqlite3_cli pthread
                                  dl)
