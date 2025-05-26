cmake_minimum_required(VERSION 3.25)
project(untitled12)

set(CMAKE_CXX_STANDARD 17)

add_executable(untitled11
        main.cpp
        Common.h
        Common.cpp)
target_link_libraries(untitled11 gdi32 user32)