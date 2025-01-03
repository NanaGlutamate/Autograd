cmake_minimum_required(VERSION 3.10)
project(AutoGrad)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)
include_directories(3rd-party)

add_executable(AutoGrad test/unique.cpp)

find_package(mimalloc CONFIG REQUIRED)
target_link_libraries(AutoGrad PRIVATE $<IF:$<TARGET_EXISTS:mimalloc-static>,mimalloc-static,mimalloc>)