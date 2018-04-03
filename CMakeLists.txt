cmake_minimum_required(VERSION 2.8)
project(sqlite_poc C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3")

option(ENABLE_COVERAGE "Enable Coverage" OFF)
if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS_DEBUG
        "${CMAKE_CXX_FLAGS_DEBUG} -pg -ftest-coverage -fprofile-arcs")
endif()

option(ENABLE_THREAD_SANITIZER "Enable ThreadSanitizer" OFF)
if(ENABLE_THREAD_SANITIZER)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
endif()

option(ENABLE_LEAK_SANITIZER "Enable LeakSanitizer" OFF)
if(ENABLE_LEAK_SANITIZER)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak")
endif()

option(ENABLE_ADDRESS_SANITIZER "Enable AddressSanitizer" OFF)
if(ENABLE_ADDRESS_SANITIZER)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(sqlite3)
add_executable(poc main.cpp
                   database.cpp
                   database_stmt.cpp
                   config.cpp)

add_executable(exec exec.cpp)

target_link_libraries(poc sqlite3)
target_link_libraries(exec sqlite3)
