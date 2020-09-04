## Root CMake file for libarcsdec

cmake_minimum_required (VERSION 3.9.6 )

## Set variables corresponding to unspecified versions to the empty string
## needed for specifying the project version along with the name in project()
cmake_policy (SET CMP0048 NEW)

## Support new IN_LIST if() operator
## needed for FindDoxygen.cmake
cmake_policy (SET CMP0057 NEW)


project (libarcsdec VERSION 0.1.0 LANGUAGES CXX )

set (PROJECT_BRIEF_DESC "Audio decoder and TOC parser adapters for libarcstk" )

set (PROJECT_URL "https://codeberg.org/tristero/${PROJECT_NAME}/" )

set (PROJECT_VERSION_SUFFIX "alpha.1" )

set (PROJECT_DEFAULT_BUILD_TYPE "Release" )


## --- Global settings

set (CMAKE_CXX_STANDARD 14 )
set (CMAKE_CXX_STANDARD_REQUIRED ON )
set (CMAKE_CXX_EXTENSIONS OFF ) ## use -std=c++14 instead of std=gnu++14


## --- Setup Version Information

set (PROJECT_VERSION_STRING_WITHOUT_SUFFIX
	"${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
	)

set (PROJECT_VERSION_STRING "${PROJECT_VERSION_STRING_WITHOUT_SUFFIX}" )

if (PROJECT_VERSION_SUFFIX)
	set(PROJECT_VERSION_STRING
		"${PROJECT_VERSION_STRING}-${PROJECT_VERSION_SUFFIX}" )
endif(PROJECT_VERSION_SUFFIX)

message (STATUS "Building ${PROJECT_NAME} ${PROJECT_VERSION_STRING}" )


## --- Setup Default Build Type

if (NOT CMAKE_BUILD_TYPE )

	set (CMAKE_BUILD_TYPE "${PROJECT_DEFAULT_BUILD_TYPE}" )

	message (STATUS "No build type specified, set to default" )

endif (NOT CMAKE_BUILD_TYPE )

message (STATUS "Build type: ${CMAKE_BUILD_TYPE}" )


## --- Project Directory Layout

## "Root directory for out-of-source build"
set (PROJECT_BINARY_DIR       "${CMAKE_CURRENT_SOURCE_DIR}/build" )

## "Root directory for public/exported headers"
set (PROJECT_INCLUDE_DIR       "${CMAKE_CURRENT_SOURCE_DIR}/include" )

## "Root directory for sources"
set (PROJECT_SOURCE_DIR       "${CMAKE_CURRENT_SOURCE_DIR}/src" )

## "Root directory for testcases and testdata"
set (PROJECT_TEST_DIR         "${CMAKE_CURRENT_SOURCE_DIR}/test" )

## "Root directory for documentation config and sources" )
set (PROJECT_DOC_DIR          "${CMAKE_CURRENT_SOURCE_DIR}/doc" )

## "Target directory for non-cmake logs"
set (PROJECT_LOG_DIR          "${PROJECT_BINARY_DIR}/log" )

## "Target directory for generated sources and configuration"
set (PROJECT_BUILD_SOURCE_DIR "${PROJECT_BINARY_DIR}/generated-sources" )

set (INSTALL_LIBRARY_DIR   "${CMAKE_INSTALL_PREFIX}/lib"
	CACHE PATH "Installation directory for library (.so) files" )

set (INSTALL_INCLUDE_DIR   "${CMAKE_INSTALL_PREFIX}/include/arcsdec"
	CACHE PATH "Installation directory for include (.hpp) files" )

set (INSTALL_PKGCONFIG_DIR "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig"
	CACHE PATH "Installation directory for pkgconfig (.pc) files" )

set (INSTALL_CMAKECONFIG_DIR
	"${CMAKE_INSTALL_PREFIX}/lib/cmake/${PROJECT_NAME}"
	CACHE PATH "Installation directory for cmake package files" )

set (CMAKE_MODULE_PATH     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules" )
message (STATUS "Changed CMAKE_MODULE_PATH to: ${CMAKE_MODULE_PATH}" )



## --- Add and configure project target

## Build libarcsdec shared library as project target

add_library (${PROJECT_NAME} SHARED
	"${PROJECT_SOURCE_DIR}/audiobuffer.cpp"
	"${PROJECT_SOURCE_DIR}/audioreader.cpp"
	"${PROJECT_SOURCE_DIR}/calculators.cpp"
	"${PROJECT_SOURCE_DIR}/descriptors.cpp"
	"${PROJECT_SOURCE_DIR}/metaparser.cpp"
	## parsercue.cpp     is required, depends on libcue
	## readerffmpeg.cpp  is optional and default ON,  depends on parts of ffmpeg
	## readerflac.cpp    is optional and default ON,  depends on FLAC
	"${PROJECT_SOURCE_DIR}/readerwav.cpp"
	## readerwvpk.cpp    is optional and default ON,  depends on libwavpack
	"${PROJECT_SOURCE_DIR}/sampleproc.cpp"
)

target_include_directories (${PROJECT_NAME}
	PUBLIC $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
	PUBLIC $<BUILD_INTERFACE:${PROJECT_BUILD_SOURCE_DIR}> ) ## version.hpp

target_compile_options (${PROJECT_NAME} PRIVATE ${PROJECT_CXX_FLAGS} )

## XXX audiobuffer? readerwav?
list (APPEND INTERFACE_HEADERS
	${PROJECT_INCLUDE_DIR}/audioreader.hpp
	${PROJECT_INCLUDE_DIR}/calculators.hpp
	${PROJECT_INCLUDE_DIR}/descriptors.hpp
	${PROJECT_INCLUDE_DIR}/metaparser.hpp
	${PROJECT_INCLUDE_DIR}/sampleproc.hpp
	${PROJECT_BUILD_SOURCE_DIR}/version.hpp )

set_target_properties (${PROJECT_NAME} PROPERTIES
	## CXX_STANDARD  14 ## Commented out: already set globally
	PUBLIC_HEADER "${INTERFACE_HEADERS}"
	PREFIX        "" ## No 'lib' prefix in soname, PROJECT_NAME is prefixed
	VERSION       "${PROJECT_VERSION_STRING_WITHOUT_SUFFIX}"
	SOVERSION     "${PROJECT_VERSION_STRING_WITHOUT_SUFFIX}" )

set (PKG_REQUIRE_ARRAY )



## --- Required library: libarcstk

## Commented out: coming with 3.14.5 ?
# set (CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE )

find_package (libarcstk 0.1.0 REQUIRED )
list (APPEND PKG_REQUIRE_ARRAY "libarcstk >= 0.1.0" )

target_link_libraries (${PROJECT_NAME} PRIVATE libarcstk::libarcstk )


## --- Configure optional features

## Default configuration for optional features

option (WITH_FLAC    "Add FLAC reading capability"     ON )
option (WITH_WAVPACK "Add WavPack reading capability"  ON )
option (WITH_FFMPEG  "Add ffmpeg reading capabilities" ON )
option (WITH_NATIVE  "Use platform specific optimization on compiling" ON )

option (IWYU "Request use of include-what-you-use" OFF )


## --- Required: parsercue (requires libcue)

find_package (libcue 2.0.0 REQUIRED )
list (APPEND PKG_REQUIRE_ARRAY "libcue >= 2.0.0" )

target_sources (${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/parsercue.cpp )
target_include_directories (${PROJECT_NAME} PRIVATE ${libcue_INCLUDE_DIRS} )
target_link_libraries (${PROJECT_NAME} PUBLIC ${libcue_LIBRARIES} )


## --- Optional: Add FLAC (requires FLAC++, default: ON)

if (WITH_FLAC )

	message (STATUS "Build with fLaC reading capability" )

	find_package (FLAC 1.3.1 REQUIRED ) ## checks libFLAC as well as libFLAC++
	list (APPEND PKG_REQUIRE_ARRAY "flac++ >= 1.3.1" )

	target_sources (${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/readerflac.cpp )
	target_include_directories (${PROJECT_NAME} PRIVATE ${FLAC_INCLUDE_DIRS} )
	target_link_libraries (${PROJECT_NAME} PUBLIC ${FLAC_LIBRARIES} )

else (WITH_FLAC)

	message (STATUS "Build without FLAC reading capability" )
endif (WITH_FLAC)


## --- Optional: Add WavPack (default: ON)

if (WITH_WAVPACK)

	message (STATUS "Build with WavPack reading capability" )

	find_package (libwavpack 5.0.0 REQUIRED )
	list (APPEND PKG_REQUIRE_ARRAY "wavpack >= 5.0.0" )

	target_sources (${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/readerwvpk.cpp )
	target_include_directories (${PROJECT_NAME} PRIVATE ${libwavpack_INCLUDE_DIRS} )
	target_link_libraries (${PROJECT_NAME} PUBLIC ${libwavpack_LIBRARIES} )

else (WITH_WAVPACK)

	message (STATUS "Build without WavPack reading capability" )
endif (WITH_WAVPACK)


## --- Optional: Add ffmpeg (default: ON)

if (WITH_FFMPEG)

	message (STATUS "Build with ffmpeg support" )

	## find_package does not respect versions for components but the ffmpeg
	## version is not trivial to access in a Find module.
	## Thus, instead of REQUIREing the lib versions, we verify them manually.

	find_package (ffmpeg REQUIRED COMPONENTS avcodec avformat avutil )

	## Verify the required versions of ffmpeg libs
	macro (at_least_version _required _actual )

		if (${_actual} VERSION_LESS ${_required})

			message (FATAL_ERROR
			"Actual version is ${_actual}, required is at least ${_required}." )

		endif()

	endmacro()

	## ffmpeg 3.1 < x < 4.0:
	## API Change: deprecated avcodec_decode_audio4() from ffmpeg 0.9 in favor
	## of avcodec_send_packet()/avcodec_receive_frame()
	at_least_version ("57.37.100" ${avcodec_VERSION}  ) ## 2016-04-21
	at_least_version ("57.33.100" ${avformat_VERSION} ) ## 2016-04-11
	at_least_version ("55.22.100" ${avutil_VERSION}   ) ## 2016-04-14
	list (APPEND PKG_REQUIRE_ARRAY "libavcodec >= 57.37.100" )
	list (APPEND PKG_REQUIRE_ARRAY "libavformat >= 57.33.100" )
	list (APPEND PKG_REQUIRE_ARRAY "libavutil >= 55.22.100" )

	## Commented out, API change is small, can be distinguished in source.
	## ffmpeg x >= 4.0:
	## API Change: deprecated av_register_all() in favor of av_muxer_iterate()
	## and av_demuxer_iterate(), registering is obsolete
	#at_least_version ("58.17.100" ${avcodec_VERSION} )  ## 2018-04-11
	#at_least_version ("58.9.100"  ${avformat_VERSION} ) ## 2018-04-01
	#at_least_version ("56.13.100" ${avutil_VERSION} )   ## 2018-04-03

	target_sources (${PROJECT_NAME}
		PRIVATE ${PROJECT_SOURCE_DIR}/readerffmpeg.cpp )

	target_include_directories (${PROJECT_NAME}
		PRIVATE ${avcodec_INCLUDE_DIRS}
		PRIVATE ${avformat_INCLUDE_DIRS}
		PRIVATE ${avutil_INCLUDE_DIRS} )

	target_link_libraries (${PROJECT_NAME}
		PUBLIC ${avcodec_LIBRARIES}
		PUBLIC ${avformat_LIBRARIES}
		PUBLIC ${avutil_LIBRARIES} )

else (WITH_FFMPEG)

	message (STATUS "Build without ffmpeg support" )
endif (WITH_FFMPEG)


## --- Configured files by CMake

## Configure all .in files producing C++ headers files

configure_file (
	"${PROJECT_INCLUDE_DIR}/version.hpp.in"
	"${PROJECT_BUILD_SOURCE_DIR}/version.hpp"
	@ONLY
)


## --- Install binary and exported header files

if (NOT SKIP_INSTALL_ALL )

	set (EXPORT_MAIN_TARGET "${PROJECT_NAME}" )

	install (TARGETS ${PROJECT_NAME}
		EXPORT "${EXPORT_MAIN_TARGET}-targets"
		LIBRARY        DESTINATION  "${INSTALL_LIBRARY_DIR}"
		PUBLIC_HEADER  DESTINATION  "${INSTALL_INCLUDE_DIR}" )

	install (EXPORT ${EXPORT_MAIN_TARGET}-targets
		NAMESPACE libarcsdec::
		DESTINATION "${INSTALL_CMAKECONFIG_DIR}" )


	if (NOT SKIP_INSTALL_FILES )

		## support pkgconfig

		string (REPLACE ";" ", " PKG_REQUIRE "${PKG_REQUIRE_ARRAY}" )

		set (PROJECT_PKGCONFIG_FILE
			"${PROJECT_BUILD_SOURCE_DIR}/libarcsdec.pc" )

		configure_file (
			"${CMAKE_CURRENT_SOURCE_DIR}/libarcsdec.pc.in"
			"${PROJECT_PKGCONFIG_FILE}"
			@ONLY
		)

		install (FILES
			"${PROJECT_PKGCONFIG_FILE}"
			DESTINATION "${INSTALL_PKGCONFIG_DIR}" )

		unset (PKG_REQUIRE )

		## support cmake

		include (CMakePackageConfigHelpers )

		set (PROJECT_CONFIG_FILE
			"${EXPORT_MAIN_TARGET}-config.cmake"
		)

		set (PROJECT_CONFIG_VERSION_FILE
			"${EXPORT_MAIN_TARGET}-config-version.cmake"
		)

		configure_package_config_file (
			"${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_CONFIG_FILE}.in"
			"${PROJECT_BUILD_SOURCE_DIR}/${PROJECT_CONFIG_FILE}"
			PATH_VARS INSTALL_INCLUDE_DIR
			INSTALL_DESTINATION "${INSTALL_CMAKECONFIG_DIR}"
		)

		write_basic_package_version_file (
			"${PROJECT_BUILD_SOURCE_DIR}/${PROJECT_CONFIG_VERSION_FILE}"
			VERSION			${PROJECT_VERSION_STRING_WITHOUT_SUFFIX}
			COMPATIBILITY	SameMajorVersion
		)

		install (FILES
			"${PROJECT_BUILD_SOURCE_DIR}/${PROJECT_CONFIG_FILE}"
			"${PROJECT_BUILD_SOURCE_DIR}/${PROJECT_CONFIG_VERSION_FILE}"
			DESTINATION "${INSTALL_CMAKECONFIG_DIR}"
		)

	endif()

endif (NOT SKIP_INSTALL_ALL)



## --- BUILD_TYPE specific settings

option (WITH_NATIVE "Use platform specific optimization on compiling" OFF )

if (CMAKE_BUILD_TYPE STREQUAL "Release")

	set (WITH_NATIVE ON CACHE BOOL
			"Release: Force platform specific optimization" FORCE )
endif ()


## --- Compiler Specific Settings

## g++: Warnings
set (PROJECT_CXX_GNU_FLAGS_WARNINGS
	-Wall -Wextra -Wpedantic -Weffc++
	-Wold-style-cast
	-Wsuggest-override
	-Wcast-align
	-Wcast-qual
	-Wctor-dtor-privacy
	-Wdisabled-optimization
	-Wformat=2
	-Wlogical-op
	-Wmissing-declarations
	-Wmissing-include-dirs
	-Wnoexcept
	-Woverloaded-virtual
	-Wredundant-decls
	-Wshadow
	-Wsign-conversion
	-Wsign-promo
	-Wstrict-null-sentinel
	-Wstrict-overflow=5
	-Wswitch-default
	-Wundef
	-Wuseless-cast
	-Wzero-as-null-pointer-constant
	)

## g++: Optimization
set (PROJECT_CXX_GNU_FLAGS_OPTIMIZE  -march=native -mtune=generic )


## clang++: Warnings
set (PROJECT_CXX_CLANG_FLAGS_WARNINGS
	-Wall -Wextra -Wpedantic -Weffc++
	-Wold-style-cast
	-Wcast-align
	-Wcast-qual
	-Wformat=2
	-Wshadow
	-Wsign-conversion
	-Wundef
	-Wunused
	-Wzero-as-null-pointer-constant
	# -- clang specific --
	-Winconsistent-missing-destructor-override
	)

## clang++: Optimization
set (PROJECT_CXX_CLANG_FLAGS_OPTIMIZE ${PROJECT_CXX_GNU_FLAGS_OPTIMIZE} )



## --- Compiler Specific Settings

## Select compiler specific settings

if (CMAKE_COMPILER_IS_GNUCXX ) ## for g++

	message (STATUS "Select compile flags for g++" )

	set (PROJECT_CXX_FLAGS_WARNINGS ${PROJECT_CXX_GNU_FLAGS_WARNINGS} )
	set (PROJECT_CXX_FLAGS_OPTIMIZE ${PROJECT_CXX_GNU_FLAGS_OPTIMIZE} )

elseif (CMAKE_CXX_COMPILER MATCHES ".*clang"       ## for clang
		OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

	message (STATUS "Select compile flags for clang++" )

	set (PROJECT_CXX_FLAGS_WARNINGS ${PROJECT_CXX_CLANG_FLAGS_WARNINGS} )
	set (PROJECT_CXX_FLAGS_OPTIMIZE ${PROJECT_CXX_CLANG_FLAGS_OPTIMIZE} )

else()

	message (WARNING "C++ Compiler seems to be neither g++ nor clang++" )
	message (WARNING "No compile flags specified for the actual C++ compiler" )
	message (WARNING "Try a generic compile without project specific flags" )

endif()


## Compiler: Activate Warnings

target_compile_options (${PROJECT_NAME} PUBLIC ${PROJECT_CXX_FLAGS_WARNINGS} )

message (STATUS "Warning Flags: ${PROJECT_CXX_FLAGS_WARNINGS}" )


## Compiler: Activate Optimizations (default ON for Release, otherwise OFF)

if (WITH_NATIVE )

	message (STATUS "Build with platform specific optimization")

	target_compile_options (${PROJECT_NAME}
		PUBLIC ${PROJECT_CXX_FLAGS_OPTIMIZE} )
else()

	message (STATUS "Build without platform specific optimization")

endif (WITH_NATIVE )

message (STATUS "Optimization Flags: ${PROJECT_CXX_FLAGS_OPTIMIZE}" )


## --- Ensure version info + current commit is available

find_package (Git QUIET REQUIRED )

## Run git with specified command options
## return $git_exit_code and $git_output
macro (RunGit )

	execute_process(COMMAND
		"${GIT_EXECUTABLE}" ${ARGV}
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
		RESULT_VARIABLE git_exit_code
		OUTPUT_VARIABLE git_output
		ERROR_QUIET
		OUTPUT_STRIP_TRAILING_WHITESPACE )

	if (git_exit_code EQUAL 0 )
		set (GIT_STATE TRUE )
	else()
		set (GIT_STATE FALSE )
	endif()
endmacro()


set(git_object HEAD)

RunGit ("describe" "--always" "${git_object}")

if (GIT_STATE )
	set (PROJECT_GIT_DESCRIBE ${git_output} )
endif()

RunGit ("rev-parse" "${git_object}" )

if (GIT_STATE )
	set (PROJECT_GIT_COMMIT "${git_output}" )
endif()


set_property(GLOBAL APPEND
	PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/.git/index"
)

message (STATUS "Git describe: ${PROJECT_GIT_DESCRIBE}" )
message (STATUS "Git commit id: ${PROJECT_GIT_COMMIT}" )


## --- Optional: Build tests, using Catch2 testing framework

option (WITH_TESTS "Build unit tests" OFF )

if (WITH_TESTS)

	message (STATUS "Build with tests" )

	enable_testing()

	## Add Catch2 test framework
	add_subdirectory (thirdparty/catch2 EXCLUDE_FROM_ALL )

	## Add tests
	add_subdirectory (${PROJECT_TEST_DIR} )

else ()

	message (STATUS "Build without tests" )
endif (WITH_TESTS)


## --- Optional: Build documentation (default: OFF)

option (WITH_DOCS          "Build documentation for public API"           OFF )
option (WITH_INTERNAL_DOCS "Build documentation also for internal APIs"   OFF )
option (WITH_IMPL_DOCS     "Build documentation also for implementations" OFF )
option (USE_MCSS           "Build documentation using m.css"              OFF )

## Auto-activate WITH_DOCS if settings for documentation are present
if (WITH_INTERNAL_DOCS OR WITH_IMPL_DOCS OR USE_MCSS )

	set(WITH_DOCS ON )

endif()

## Process configuration for building the documentation
if (WITH_DOCS )

	add_subdirectory(${PROJECT_DOC_DIR} )

endif (WITH_DOCS )


## --- Optional: Add include-what-you-use support (default: OFF)

if (IWYU )

	message (STATUS "Program include-what-you-use requested" )
	find_program (IWYU_PATH NAMES include-what-you-use iwyu )

	if (IWYU_PATH )

		## Set for all subsequent targets
		set (CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PATH} )

		## Commented out: Set for specific targets only
		#set_property(TARGET ${TARGET_NAME}
		#	PROPERTY CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PATH})

		message (STATUS "Program include-what-you-use found: ${IWYU_PATH}" )

	else (IWYU_PATH )

		message (WARNING "Program include-what-you-use not found" )

	endif (IWYU_PATH )
endif (IWYU )


## --- Create Log directory

file (MAKE_DIRECTORY ${PROJECT_LOG_DIR} )

