## Create a test executable from all .cpp files in a directory
## and register tests found with catch_discover_tests.

## Link libraries of _feature_target in _test_target without introducing
## a dependency between the targets.
function (link_feature_deps _test_target _feature_target)

	message (STATUS "Link target ${_test_target} "
		"to the dependencies of target ${_feature_target}" )

	get_target_property (_feature_libs ${_feature_target} LINK_LIBRARIES )

	if (_feature_libs)

		target_link_libraries (${_test_target} PRIVATE ${_feature_libs} )
	endif ()
endfunction()

##
## Usage:
##   add_test_suite(unit
##       LABEL "unit"
##       TIMEOUT 10
##   )
function (add_test_suite CATEGORY )

	## Collect all test source files in src/ directory
	file (GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" )
	if (NOT TEST_SOURCES )
		message (FATAL_ERROR
			"No test sources found in ${CMAKE_CURRENT_SOURCE_DIR}/src/" )
	endif()

	## Arguments
	set (one_value_args LABEL TIMEOUT DEPS_FROM )
	cmake_parse_arguments (SUITE "" "${one_value_args}" "" ${ARGN} )

	#message (STATUS "Found tests of category '${CATEGORY}': ${TEST_SOURCES}" )

	set (SUITE_NAME "${CATEGORY}_tests" )

	#message (STATUS "CATEGORY: '${CATEGORY}'" )
	#message (STATUS "SUITE_NAME: '${SUITE_NAME}'" )
	#message (STATUS "SUITE_DEPS_FROM: '${SUITE_DEPS_FROM}'" )

	## Create executable
	add_executable (${SUITE_NAME} ${TEST_SOURCES} )

	## Standard configuration
	set_target_properties (${SUITE_NAME} PROPERTIES
		CXX_STANDARD           17
		CXX_STANDARD_REQUIRED  ON
		BUILD_RPATH            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
		BUILD_RPATH_USE_ORIGIN ON
		SKIP_RPATH             OFF
	)

	target_compile_options (${SUITE_NAME}
		PRIVATE ${TEST_CXX_FLAGS_WARNINGS}
		PRIVATE ${LIBARCSDEC_CXX_FLAGS_OPTIMIZE}
	)

	## Include paths
	target_include_directories (${SUITE_NAME}
		PRIVATE "${LIBARCSDEC_INCLUDE_SOURCE_DIR}"        ## public headers
		PRIVATE "${LIBARCSDEC_SOURCE_DIR}"                ## private headers
		PRIVATE "${LIBARCSDEC_SOURCE_DIR}/${SUITE_LABEL}" ## private headers
		PRIVATE "${LIBARCSDEC_ROOT_DIR}/test/features/include"## private headers
		PRIVATE "${LIBARCSDEC_GENSRC_BINARY_DIR}"         ## generated sources
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include"     ## test suite includes
	)

	unset (_include )

	## Link libraries
	target_link_libraries (${SUITE_NAME}
		PRIVATE Catch2::Catch2WithMain
		PRIVATE ${PROJECT_NAME} ## libarcsdec from build-tree
	)

	## Get deps from specified target
	if (TARGET ${SUITE_DEPS_FROM} )

		## We do NOT just make SUITE_DEPS_FROM a dependency of SUITE_NAME
		## because SUITE_DEPS_FROM will already be linked against the
		## main target and the main target deps - to which also the test will
		## link. This multiplicity may cause ODR violations and other problems
		## within the build.
		link_feature_deps (${SUITE_NAME} ${SUITE_DEPS_FROM} )
	endif()

	target_link_libraries (${SUITE_NAME} PRIVATE libarcstk::libarcstk )

	## Set properties for all discovered tests

	set (TEST_PROPERTIES)

	if (SUITE_LABEL )
		list (APPEND TEST_PROPERTIES LABELS ${SUITE_LABEL} )
	endif()

	if (SUITE_TIMEOUT )
		list (APPEND TEST_PROPERTIES TIMEOUT ${SUITE_TIMEOUT} )
	endif()


	## Discover and register all tests from the executable to CTest
	catch_discover_tests (${SUITE_NAME}
		TEST_PREFIX       "${CATEGORY}/"
		REPORTER          "junit"
		OUTPUT_DIR        "${LIBARCSDEC_BINARY_DIR}/reports"
		OUTPUT_PREFIX     "report."
		OUTPUT_SUFFIX     ".xml"
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		PROPERTIES        ${TEST_PROPERTIES}
	)
endfunction()

