## CMake file for building features of libarcsdec
## vim:fdm=marker

cmake_minimum_required (VERSION 3.10 )

## -- Functionality for features: flex + bison {{{1

find_package (BISON ) # 3.8.2
find_package (FLEX  ) # 2.6.4

function (add_bison_command )

	## Arguments
	set (one_value_args NAME OUTPUT_CPP OUTPUT_HPP Y_FILE FLAGS WORKING_DIR )
	cmake_parse_arguments (BISON "" "${one_value_args}" "" ${ARGN} )

	add_custom_command (
		OUTPUT ${BISON_OUTPUT_CPP} ${BISON_OUTPUT_HPP}
		COMMAND bison ${BISON_FLAGS}
			--defines=${BISON_OUTPUT_HPP} -o ${BISON_OUTPUT_CPP} ${BISON_Y_FILE}
		COMMENT "Generating parser for format ${BISON_NAME} (via bison)"
		WORKING_DIRECTORY ${BISON_WORKING_DIR}
	)
endfunction()

function (add_flex_command )

	## Arguments
	set (one_value_args NAME OUTPUT_CPP L_FILE INPUT_HEADER FLAGS WORKING_DIR )
	cmake_parse_arguments (FLEX "" "${one_value_args}" "" ${ARGN} )

	add_custom_command (
		OUTPUT ${FLEX_OUTPUT_CPP}
		COMMAND flex ${FLEX_FLAGS}
			-o ${FLEX_OUTPUT_CPP} ${FLEX_L_FILE}
		DEPENDS ${FLEX_INPUT_HEADER}
		COMMENT "Generating lexer for format ${FLEX_NAME} (via flex)"
		WORKING_DIRECTORY ${FLEX_WORKING_DIR}
	)
endfunction()

# 1}}}

## -- Internal features: readerwav, parsercue, parsertoc {{{1

foreach (_filereader IN ITEMS "readerwav" "parsercue" "parsertoc" )

	add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/${_filereader} )
endforeach()
# 1}}}

## -- Optional features

option (WITH_FLAC       "Add FLAC reading capability"          ON )
option (WITH_FFMPEG     "Add ffmpeg reading capabilities"      ON )
option (WITH_WAVPACK    "Add WavPack reading capability"       ON )
option (WITH_LIBSNDFILE "Add libsndfile reading capabilities" OFF )
option (WITH_LIBCUE     "Add libcue parsing capability"       OFF )

# -- Optional features: reader{flac,ffmpeg,wvpk,sndfile}, parserlibcue

## --- Optional: FLAC (default: ON) {{{2

if (WITH_FLAC )

	add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/readerflac )

	readerflac_pkg (_readerflac_deps )
	list (APPEND ${PKG_LIST} ${_readerflac_deps} )

else (WITH_FLAC)

	message (STATUS "Build without FLAC reading capability" )
endif (WITH_FLAC)
# 2}}}

## --- Optional: ffmpeg (default: ON) {{{2

if (WITH_FFMPEG)

	add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/readerffmpeg )

	readerffmpeg_pkg (_readerffmpeg_deps )
	list (APPEND ${PKG_LIST} ${_readerffmpeg_deps} )

else (WITH_FFMPEG)

	message (STATUS "Build without ffmpeg support" )
endif (WITH_FFMPEG)
# 2}}}

## --- Optional: WavPack (default: ON) {{{2

if (WITH_WAVPACK)

	add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/readerwvpk )

	readerwvpk_pkg (_readerwvpk_deps )
	list (APPEND ${PKG_LIST} ${_readerwvpk_deps} )

else (WITH_WAVPACK)

	message (STATUS "Build without WavPack reading capability" )
endif (WITH_WAVPACK)
# 2}}}

## --- Optional: libsndfile (default: OFF) {{{2

if (WITH_LIBSNDFILE )

	add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/readersndfile )

	readersndfile_pkg (_readersndfile_deps )
	list (APPEND ${PKG_LIST} ${_readersndfile_deps} )

else (WITH_LIBSNDFILE )

	message (STATUS "Build without libsndfile support" )
endif (WITH_LIBSNDFILE )
# 2}}}

## --- Optional: parserlibcue (default: OFF) {{{2

if (WITH_LIBCUE )

	add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/parserlibcue )

	parserlibcue_pkg (_parserlibcue_deps )
	list (APPEND ${PKG_LIST} ${_parserlibcue_deps} )

else (WITH_LIBCUE)

	message (STATUS "Build without libcue parsing capability" )
endif (WITH_LIBCUE)
# 2}}}

set (${PKG_LIST} "${${PKG_LIST}}" PARENT_SCOPE )

