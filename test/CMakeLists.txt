## CMake file for configuring and running tests

cmake_minimum_required (VERSION 3.7 )

## Always link library files by full path when a full path is given to the
## target_link_libraries() command.
cmake_policy (SET CMP0003 NEW )

## Link libraries by full path even in implicit directories.
cmake_policy (SET CMP0060 NEW )


set (TEST_SOURCE_DIR "src" )


## Define testsuites

set (TEST_SETS )
list (APPEND TEST_SETS audioreader  )
list (APPEND TEST_SETS calculators  )
list (APPEND TEST_SETS descriptor   )
list (APPEND TEST_SETS libinspect   )
list (APPEND TEST_SETS cuesheet.yy  ) # details of parsercue
list (APPEND TEST_SETS readerwav    )
list (APPEND TEST_SETS selection    )
list (APPEND TEST_SETS dec_version  )

if (WITH_LIBCUE )
list (APPEND TEST_SETS parserlibcue )
endif()

if (WITH_LIBCDIO )
list (APPEND TEST_SETS parsertoc    )
endif()

if (WITH_FLAC )
list (APPEND TEST_SETS readerflac   )
endif()

if (WITH_WAVPACK )
list (APPEND TEST_SETS readerwvpk   )
endif()

if (WITH_FFMPEG )
list (APPEND TEST_SETS readerffmpeg )
endif()

if (WITH_LIBSNDFILE )
list (APPEND TEST_SETS readersndfile )
endif()


## Adjust compile options for tests.
## (g++ issues many warnings when compiling Catch2 v3.x tests)

set (TEST_CXX_FLAGS_WARNINGS ${PROJECT_CXX_FLAGS_WARNINGS} )


## Remove flags from g++ that pollute the output

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


## Add common configuration for all testsuites (catch2 + project dependencies)

foreach (_testcase ${TEST_SETS} )

	set_property (
		SOURCE "${TEST_SOURCE_DIR}/${_testcase}.cpp"
		APPEND
		PROPERTY COMPILE_OPTIONS ${TEST_CXX_FLAGS_WARNINGS}
	)

	## Define an executable for each single testfile
	add_executable (${_testcase}_test "${TEST_SOURCE_DIR}/${_testcase}.cpp" )

	set_property (TARGET ${_testcase}_test PROPERTY CXX_STANDARD 14 )

	target_include_directories (${_testcase}_test
		PRIVATE ${PROJECT_INCLUDE_DIR}      ## public headers
		PRIVATE ${PROJECT_SOURCE_DIR}       ## private headers (*_details.hpp)
		PRIVATE ${PROJECT_BUILD_SOURCE_DIR} ## generated sources
	)

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

	target_compile_options (${_testcase}_test
		PRIVATE ${PROJECT_CXX_FLAGS_OPTIMIZE} )

	## Add test to ctest set
	add_test (
		NAME    ${_testcase}_test
		COMMAND ${_testcase}_test
			-o "${PROJECT_BINARY_DIR}/report.${_testcase}.xml"
			-r junit
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data"
	)

endforeach()

## Add libcue dependencies to testcase
if (WITH_LIBCUE )

	target_include_directories (parserlibcue_test
		PRIVATE ${libcue_INCLUDE_DIRS} )

	target_link_libraries (parserlibcue_test PRIVATE ${libcue_LIBRARIES} )
endif()

## Add FLAC dependencies to testcase
if (WITH_FLAC )

	if (TARGET FLAC::FLAC++ )
		target_link_libraries (readerflac_test PRIVATE FLAC::FLAC++ )
	else()
		target_include_directories (readerflac_test
			PRIVATE ${LIBFLACPP_INCLUDE_DIRS} )
		target_link_libraries (readerflac_test
			PRIVATE ${LIBFLACPP_LIBRARIES} )
	endif()
endif()

