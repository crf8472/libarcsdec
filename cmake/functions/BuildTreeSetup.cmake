## libarcsdec: Setup build tree
## vim:fdm=marker

function (setup_build_tree)

## -- Log Directory {{{1

add_custom_command (
	OUTPUT  "${LIBARCSDEC_LOG_BINARY_DIR}"
	COMMAND "${CMAKE_COMMAND}"
	ARGS    -E make_directory "${LIBARCSDEC_LOG_BINARY_DIR}"
	VERBATIM
)

add_custom_target (libarcsdec_create_log_dir ALL
	DEPENDS "${LIBARCSDEC_LOG_BINARY_DIR}" )
##}}}1

## -- Build tree include Directory + Link {{{1

add_custom_command (
	OUTPUT  "${LIBARCSDEC_BINARY_DIR}/include"
	COMMAND "${CMAKE_COMMAND}"
	ARGS    -E make_directory "${LIBARCSDEC_BINARY_DIR}/include"
	VERBATIM
)

add_custom_target (libarcsdec_create_include_dir
	DEPENDS "${LIBARCSDEC_BINARY_DIR}/include"
	VERBATIM
)

## Create build-tree include directory (symlink)
## This is useful for finding the includes when used as a subproject.
add_custom_target (libarcsdec_link_to_headers
	COMMAND "${CMAKE_COMMAND}"
		-E create_symlink
		"${LIBARCSDEC_INCLUDE_SOURCE_DIR}"
		"${LIBARCSDEC_INCLUDE_BINARY_DIR}"
	VERBATIM
)

add_dependencies (libarcsdec_link_to_headers libarcsdec_create_include_dir )
##}}}1

endfunction()

