# The recommended cmake version is at least 3.20
if(TARGET cepo::cepo)
	return()
endif()

function(cepo_dependent_integrate)
	get_filename_component(
		cepo_root
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}"
		ABSOLUTE
	)

	add_subdirectory(
		"${cepo_root}"
		"${CMAKE_CURRENT_BINARY_DIR}/cepo_build"
		EXCLUDE_FROM_ALL
	)
endfunction()

cepo_dependent_integrate()
