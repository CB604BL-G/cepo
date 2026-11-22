cmake_minimum_required(VERSION 3.20)

project(SCOPE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(scope test.cpp)

target_include_directories(scope PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../../../../include"
)

target_link_libraries(scope PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../../../gen_lib/libcepo.a"
)
