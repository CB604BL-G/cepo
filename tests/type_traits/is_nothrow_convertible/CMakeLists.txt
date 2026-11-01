cmake_minimum_required(VERSION 3.20)

project(IS_NOTHROW_CONVERTIBLE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(is_nothrow_convertible test.cpp)

target_include_directories(is_nothrow_convertible PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../../../include"
)

target_link_libraries(is_nothrow_convertible PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../../gen_lib/libcepo.a"
)
