cmake_minimum_required(VERSION 3.20)

project(INTERFACE_STATE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(interface_state test.cpp)

target_include_directories(interface_state PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../../../../include"
)

target_link_libraries(interface_state PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../../../gen_lib/libcepo.a"
)
