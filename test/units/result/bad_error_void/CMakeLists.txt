cmake_minimum_required(VERSION 3.20)

project(BAD_ERROR_VOID LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(bad_error_void test.cpp)

target_include_directories(bad_error_void PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../../../../include"
)

target_link_libraries(bad_error_void PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../../../gen_lib/libcepo.a"
)
