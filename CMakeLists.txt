cmake_minimum_required(VERSION 3.21)
project(mandelbrot_set_v2 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Catch2 3 REQUIRED)
find_package(trompeloeil REQUIRED)

add_executable(mandelbrot src/main.cpp)
target_include_directories(mandelbrot PRIVATE include)

enable_testing()
add_subdirectory(tests)
