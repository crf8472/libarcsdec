## CMake file for managing libarcsdec dependencies
## vim:fdm=marker

set (_libarcstk_MINIMUM_REQUIRED_VERSION "9.0.0" )

function (libarcsdec_setup_required_dependencies PKG_LIST )
## -- Required dependency: libarcstk {{{1
if (WITH_SUBMODULES )

	## Avoid WITH_TESTS and WITH_EXAMPLES just falling through to libarcstk
	option (WITH_LIBARCSTK_TESTS    "Build unit tests of libarcstk" OFF )
	option (WITH_LIBARCSTK_EXAMPLES "Build examples of libarcstk"   OFF )

	if (NOT HAS_PARENT )

		message (STATUS "Link to local submodule libarcstk" )

		## If not set by some PARENT, just use local subdir
		if (NOT SUBMODULES_DIR )
			set (SUBMODULES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs" )
		endif()

		## backup original options
		set (WITH_LIBARCSDEC_TESTS    ${WITH_TESTS}
			CACHE BOOL "Backup value of CLI option -DWITH_TESTS" FORCE )
		set (WITH_LIBARCSDEC_EXAMPLES ${WITH_EXAMPLES}
			CACHE BOOL "Backup value of CLI option -DWITH_EXAMPLES" FORCE )

		## override + deactivate options for libarcstk
		set (WITH_TESTS    ${WITH_LIBARCSTK_TESTS}
			CACHE BOOL "Override value of CLI option -DWITH_TESTS for libarcstk"
			FORCE )
		set (WITH_EXAMPLES ${WITH_LIBARCSTK_EXAMPLES}
			CACHE BOOL
				"Override value of CLI option -DWITH_EXAMPLES for libarcstk"
			FORCE )

		add_subdirectory (${SUBMODULES_DIR}/libarcstk )

		## restore original options
		set (WITH_TESTS    ${WITH_LIBARCSDEC_TESTS}
			CACHE BOOL "Build unit tests" FORCE )
		set (WITH_EXAMPLES ${WITH_LIBARCSDEC_EXAMPLES}
			CACHE BOOL "Build examples" FORCE )

	endif()

	## If PARENT is present, do nothing. PARENT has just to add_subdirectory()
	## and target_link_libraries().

else()

	find_package (libarcstk ${_libarcstk_MINIMUM_REQUIRED_VERSION} REQUIRED )

	list (APPEND PKG_LIST "libarcstk >= ${_libarcstk_MINIMUM_REQUIRED_VERSION}"
		PARENT_SCOPE )

endif (WITH_SUBMODULES )


if (TARGET libarcstk::libarcstk ) ## from PARENT, submodule or system

	target_link_libraries (${PROJECT_NAME} PRIVATE libarcstk::libarcstk )
else()

	message (FATAL_ERROR
		"libarcstk is not present, neither system-wide nor as a submodule" )
endif()
# 1}}}
endfunction()

