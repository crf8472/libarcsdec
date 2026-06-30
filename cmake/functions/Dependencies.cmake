## CMake file for managing libarcsdec dependencies
## vim:fdm=marker

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

	find_package (libarcstk 0.9.0 REQUIRED )

	list (APPEND PKG_LIST "libarcstk >= 0.9.0" PARENT_SCOPE )
endif (WITH_SUBMODULES )


if (TARGET libarcstk::libarcstk ) ## from PARENT, submodule or system

	target_link_libraries (${PROJECT_NAME} PRIVATE libarcstk::libarcstk )
else()

	message (FATAL_ERROR
		"libarcstk is not present, neither system-wide nor as a submodule" )
endif()
# 1}}}
endfunction()

function (libarcsdec_setup_internal_dependencies )
## -- Required internal features: readerwav, parsercue, parsertoc {{{1

foreach (_filereader IN ITEMS "readerwav" "parsercue" "parsertoc" )

	add_subdirectory (${LIBARCSDEC_SOURCE_DIR}/${_filereader} )
endforeach()
# 1}}}
endfunction()

## -- Optional features {{{1

option (WITH_LIBCUE     "Add libcue parsing capability"       OFF )
option (WITH_FFMPEG     "Add ffmpeg reading capabilities"      ON )
option (WITH_FLAC       "Add FLAC reading capability"          ON )
option (WITH_LIBSNDFILE "Add libsndfile reading capabilities" OFF )
option (WITH_WAVPACK    "Add WavPack reading capability"       ON )

function (libarcsdec_setup_configured_dependencies PKG_LIST )

## --- Optional: parserlibcue (requires libcue, default: OFF) {{{2

if (WITH_LIBCUE )

	add_subdirectory (${LIBARCSDEC_SOURCE_DIR}/parserlibcue )

	parserlibcue_pkg (_parserlibcue_deps )
	list (APPEND ${PKG_LIST} ${_parserlibcue_deps} )

else (WITH_LIBCUE)

	message (STATUS "Build without libcue parsing capability" )
endif (WITH_LIBCUE)
# 2}}}

## --- Optional: FLAC (requires FLAC++, default: ON) {{{2

if (WITH_FLAC )

	add_subdirectory (${LIBARCSDEC_SOURCE_DIR}/readerflac )

	readerflac_pkg (_readerflac_deps )
	list (APPEND ${PKG_LIST} ${_readerflac_deps} )

else (WITH_FLAC)

	message (STATUS "Build without FLAC reading capability" )
endif (WITH_FLAC)
# 2}}}

## --- Optional: WavPack (default: ON) {{{2

if (WITH_WAVPACK)

	add_subdirectory (${LIBARCSDEC_SOURCE_DIR}/readerwvpk )

	readerwvpk_pkg (_readerwvpk_deps )
	list (APPEND ${PKG_LIST} ${_readerwvpk_deps} )

else (WITH_WAVPACK)

	message (STATUS "Build without WavPack reading capability" )
endif (WITH_WAVPACK)
# 2}}}

## --- Optional: ffmpeg (default: ON) {{{2

if (WITH_FFMPEG)

	add_subdirectory (${LIBARCSDEC_SOURCE_DIR}/readerffmpeg )

	readerffmpeg_pkg (_readerffmpeg_deps )
	list (APPEND ${PKG_LIST} ${_readerffmpeg_deps} )

else (WITH_FFMPEG)

	message (STATUS "Build without ffmpeg support" )
endif (WITH_FFMPEG)
# 2}}}

## --- Optional: libsndfile (default: ON) {{{2

if (WITH_LIBSNDFILE )

	add_subdirectory (${LIBARCSDEC_SOURCE_DIR}/readersndfile )

	readersndfile_pkg (_readersndfile_deps )
	list (APPEND ${PKG_LIST} ${_readersndfile_deps} )

else (WITH_LIBSNDFILE )

	message (STATUS "Build without libsndfile support" )
endif (WITH_LIBSNDFILE )
# 2}}}

	set (${PKG_LIST} "${${PKG_LIST}}" PARENT_SCOPE )
endfunction()
# 1}}}

