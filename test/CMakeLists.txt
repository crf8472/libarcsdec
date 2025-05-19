## CMake build script for configuring and building tests
##
## vim:fdm=marker

cmake_minimum_required (VERSION 3.10 )
## --- Policies {{{1

## Always link library files by full path when a full path is given to the
## target_link_libraries() command.
cmake_policy (SET CMP0003 NEW )

## Link libraries by full path even in implicit directories.
## See: https://cmake.org/cmake/help/latest/policy/CMP0060.html
cmake_policy (SET CMP0060 NEW )
## 1}}}



## --- Define testcases {{{1

set (TEST_SETS )

## Mandatory sources
list (APPEND TEST_SETS audioreader           )
list (APPEND TEST_SETS calculators           )
list (APPEND TEST_SETS descriptor            )
list (APPEND TEST_SETS libinspect            )
list (APPEND TEST_SETS parsercue             )
list (APPEND TEST_SETS parsercue_details     )
list (APPEND TEST_SETS readerwav             )
list (APPEND TEST_SETS readerwav_details     )
list (APPEND TEST_SETS selection             )
list (APPEND TEST_SETS dec_version           )

if (WITH_LIBCUE )
	list (APPEND TEST_SETS parserlibcue          )
	list (APPEND TEST_SETS parserlibcue_details  )
endif()

if (WITH_FFMPEG )
	list (APPEND TEST_SETS readerffmpeg          )
	list (APPEND TEST_SETS readerffmpeg_details  )
endif()

if (WITH_FLAC )
	list (APPEND TEST_SETS readerflac            )
	list (APPEND TEST_SETS readerflac_details    )
endif()

if (WITH_WAVPACK )
	list (APPEND TEST_SETS readerwvpk            )
	list (APPEND TEST_SETS readerwvpk_details    )
endif()

if (WITH_LIBSNDFILE )
	list (APPEND TEST_SETS readersndfile         )
	list (APPEND TEST_SETS readersndfile_details )
endif()



## --- Adjust compile options for tests {{{1
## (g++ issues many warnings when compiling Catch2 v3.x tests)

set (TEST_CXX_FLAGS_WARNINGS ${PROJECT_CXX_FLAGS_WARNINGS} )



## --- Remove flags from g++ that pollute the output {{{1

if (CMAKE_COMPILER_IS_GNUCXX )

	## Remove some flags from the actual compile options, if they are present
	list (APPEND GNUCXX_IGNORE_FLAGS
		-Weffc++
		-Wctor-dtor-privacy
		-Wuseless-cast
	)

	foreach (_flag ${GNUCXX_IGNORE_FLAGS} )
		message (STATUS
			"Ignore compile flag ${_flag} when compiling Catch2-tests with g++" )
		list (REMOVE_ITEM TEST_CXX_FLAGS_WARNINGS ${_flag} )
	endforeach()
endif()



## --- Add common configuration for all testcases {{{1

foreach (_testcase ${TEST_SETS} )

	## Use test/src/ by default as include directory for the testcase
	set (COMPONENT_TEST_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src" )

	## Derive subdirectory name to find in src/ and test/src/
	string (REPLACE _details ""  SOURCES_SUBDIR  "${_testcase}" )

	## Check for subdirectory and add it to a loadlist, if it exists
	get_filename_component (_fullpath
		"${COMPONENT_TEST_SOURCE_DIR}/${SOURCES_SUBDIR}" REALPATH )
	if (EXISTS "${_fullpath}" )
		## Update default since we found a specific subdirectory
		set (COMPONENT_TEST_SOURCE_DIR
			"${COMPONENT_TEST_SOURCE_DIR}/${SOURCES_SUBDIR}" )

		## Add subdirectory to loadlist
		if (EXISTS "${COMPONENT_TEST_SOURCE_DIR}/CMakeLists.txt" )
			if (NOT "${COMPONENT_TEST_SOURCE_DIR}" IN_LIST LOAD_DIR_LIST )
				list (APPEND LOAD_DIR_LIST "${COMPONENT_TEST_SOURCE_DIR}" )
			endif ()
		endif ()
	endif ()
	unset (_fullpath )


	## Use src/ by default as include directory for the sources
	set (COMPONENT_SOURCE_DIR "${PROJECT_SOURCE_DIR}")

	## Check for subdirectory and use it, if it exists
	get_filename_component (_fullpath "${PROJECT_SOURCE_DIR}/${SOURCES_SUBDIR}"
		REALPATH )
	if (EXISTS "${_fullpath}" )
		set (COMPONENT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/${SOURCES_SUBDIR}")
	endif ()


	## Set compile flags for testing
	set_property (
		SOURCE "${COMPONENT_TEST_SOURCE_DIR}/${_testcase}.cpp"
		APPEND
		PROPERTY COMPILE_OPTIONS ${TEST_CXX_FLAGS_WARNINGS}
	)

	## Define executable for this testcase
	add_executable (${_testcase}_test
		"${COMPONENT_TEST_SOURCE_DIR}/${_testcase}.cpp" )

	set_property (TARGET ${_testcase}_test PROPERTY CXX_STANDARD 17 )

	target_include_directories (${_testcase}_test
		PRIVATE ${PROJECT_INCLUDE_BINARY_DIR}      ##  public libarcsdec headers
		PRIVATE ${COMPONENT_SOURCE_DIR}            ## private libarcsdec headers
		PRIVATE ${PROJECT_GEN_SOURCES_DIR}         ## generated sources
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include" ## test headers
	)

	unset (COMPONENT_TEST_SOURCE_DIR )
	unset (COMPONENT_SOURCE_DIR )

	target_link_libraries (${_testcase}_test
		PRIVATE
		Catch2::Catch2WithMain
		libarcstk::libarcstk
		-Wl,--disable-new-dtags  ## set RPATH instead of RUNPATH
		${PROJECT_NAME} ## libarcsdec from build-tree, 0003 and 0060 are used
	)
	## Setting the RPATH forces loading the libarcstk.so from the build tree,
	## even if a system-wide libarcstk is installed. Using RUNPATH instead of
	## RPATH causes the testcase to load the system-wide libfile.

	## Set same optimization flags as for project sources
	target_compile_options (${_testcase}_test
		PRIVATE ${PROJECT_CXX_FLAGS_OPTIMIZE} )

	## Add test to ctest set
	add_test (
		NAME    ${_testcase}_test
		COMMAND "${_testcase}_test"
				-o "${PROJECT_BINARY_DIR}/report.${_testcase}.xml"
				-r junit
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data"
	)

endforeach()



## --- Process build scripts for testcases {{{1
foreach (DIR ${LOAD_DIR_LIST} )
	add_subdirectory (${DIR} )
endforeach()

