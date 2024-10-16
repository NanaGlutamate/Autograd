cmake_minimum_required(VERSION 3.10)
project(AutoGrad)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

add_executable(AutoGrad test/unique.cpp)