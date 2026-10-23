cmake_minimum_required(VERSION 3.20)
project(test_dependency_of_cepo LANGUAGES CXX)

include("$ENV{HOME}/.local/share/cepo/lib.cmake")

add_executable(test_dependency_of_cepo test.cpp)
target_link_libraries(test_dependency_of_cepo PRIVATE cepo::cepo)
