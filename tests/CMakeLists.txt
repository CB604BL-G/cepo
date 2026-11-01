cmake_minimum_required(VERSION 3.20)

project(cepo_test_build LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Place libcepo.a directly under tests/gen_lib/
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/gen_lib")

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/.." cepo_build)
