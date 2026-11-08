cmake_minimum_required(VERSION 3.20)
project(dependency_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include("$ENV{CEPO_CMAKE}")

add_executable(dependency_test test.cpp)
target_link_libraries(dependency_test PRIVATE cepo::cepo)
