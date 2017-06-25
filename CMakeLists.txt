cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

set(CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS} -O0 -g3 -pg -ftest-coverage -fprofile-arcs")

include_directories($ENV{BOOST_ROOT})
include_directories(sqlite3)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(sqlite3)
add_executable(poc main.cpp
                   config.cpp)

add_executable(exec exec.cpp)

target_link_libraries(poc sqlite3)
target_link_libraries(exec sqlite3)
