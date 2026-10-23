cmake_minimum_required(VERSION 3.20)

project(
	cepo
	LANGUAGES CXX
)

add_library(
	cepo_library
	"src/result/bad_result_unwrap.cpp"
)

set_target_properties(
	cepo_library PROPERTIES
	OUTPUT_NAME cepo
)

add_library(
	cepo::cepo
	ALIAS cepo_library
)

target_compile_features(
	cepo_library
	PUBLIC cxx_std_11
)

target_include_directories(
	cepo_library
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
