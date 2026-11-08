cmake_minimum_required(VERSION 3.20)
project(quick_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/.." cepo_build)

add_executable(quick_test main.cpp)
target_link_libraries(quick_test PRIVATE cepo::cepo)
