## Root CMake file for libarcsdec

cmake_minimum_required (VERSION 3.9.6 )

## Set variables corresponding to unspecified versions to the empty string
## needed for specifying the project version along with the name in project()
cmake_policy (SET CMP0048 NEW)

## Support new IN_LIST if() operator
## needed for FindDoxygen.cmake
cmake_policy (SET CMP0057 NEW)


project (arcsdec VERSION 0.1.0 LANGUAGES CXX )

set (PROJECT_BRIEF_DESC
    "A library with audio decoder and TOC parser adapters for libarcstk" )

set (PROJECT_URL "https://codeberg.org/tristero/libarcsdec/" )

set (PROJECT_VERSION_SUFFIX "alpha.1" )

set (PROJECT_DEFAULT_BUILD_TYPE "Release" )


## --- Setup Version Information

set (PROJECT_VERSION_STRING_WITHOUT_SUFFIX
	"${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
	)

set (PROJECT_VERSION_STRING "${PROJECT_VERSION_STRING_WITHOUT_SUFFIX}" )

if (PROJECT_VERSION_SUFFIX)
	set(PROJECT_VERSION_STRING
		"${PROJECT_VERSION_STRING}-${PROJECT_VERSION_SUFFIX}" )
endif(PROJECT_VERSION_SUFFIX)

message (STATUS "Building lib${PROJECT_NAME} ${PROJECT_VERSION_STRING}" )


## --- Setup Default Build Type

if (NOT CMAKE_BUILD_TYPE )

	set (CMAKE_BUILD_TYPE "${PROJECT_DEFAULT_BUILD_TYPE}"
		CACHE STRING "Choose the type of build (e.g. Debug or Release)" FORCE )

	message (STATUS "No build type specified, set to default" )

endif (NOT CMAKE_BUILD_TYPE )

message (STATUS "Build type: ${CMAKE_BUILD_TYPE}" )


## --- Project Directory Layout

set (PROJECT_BINARY_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/build" )
set (PROJECT_SOURCE_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/src" )
set (PROJECT_TEST_DIR       "${CMAKE_CURRENT_SOURCE_DIR}/test" )
set (PROJECT_DOC_DIR        "${CMAKE_CURRENT_SOURCE_DIR}/doc" )

set (INSTALL_LIBRARY_DIR   "${CMAKE_INSTALL_PREFIX}/lib"
	CACHE PATH "Installation directory for library (.so) files" )

set (INSTALL_INCLUDE_DIR   "${CMAKE_INSTALL_PREFIX}/include"
	CACHE PATH "Installation directory for include (.hpp) files" )

set (INSTALL_PKGCONFIG_DIR "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig"
	CACHE PATH "Installation directory for pkgconfig (.pc) files" )

set (CMAKE_MODULE_PATH     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules" )
message (STATUS "Changed CMAKE_MODULE_PATH to: ${CMAKE_MODULE_PATH}" )


## --- Add Sources

## Add all source files without any dependencies except C++ STL.
## Source files with extern dependencies are added later as object libraries.

list (APPEND SOURCES "${PROJECT_SOURCE_DIR}/calculators.cpp" )

list (APPEND SOURCES "${PROJECT_SOURCE_DIR}/audiobuffer.cpp" )
list (APPEND SOURCES "${PROJECT_SOURCE_DIR}/audioreader.cpp" )
list (APPEND SOURCES "${PROJECT_SOURCE_DIR}/descriptors.cpp" )
list (APPEND SOURCES "${PROJECT_SOURCE_DIR}/metaparser.cpp" )
## parserdev.cpp     is optional and default OFF, depends on libcdio
## parsercue.cpp     is required, depends on libcue
## readerffmpeg.cpp  is optional and default ON,  depends on parts of ffmpeg
## readerflac.cpp    is optional and default ON,  depends on FLAC
## readersndfile.cpp is optional and default OFF, depends on libsndfile
list (APPEND SOURCES "${PROJECT_SOURCE_DIR}/readerwav.cpp" )
## readerwvpk.cpp    is optional and default ON,  depends on libwavpack
list (APPEND SOURCES "${PROJECT_SOURCE_DIR}/sampleproc.cpp" )


## --- Define required objects and libraries to link to the project target

set (REQUIRED_OBJECTS )

set (REQUIRED_LIBRARIES )


## --- Define compiler switches

set (FLAGS_WARNINGS -Wall -Wextra -Wold-style-cast -Wpedantic -Weffc++ )

set (FLAGS_OPTIMIZE -march=native -mtune=generic )


## --- Required library: libarcstk

find_package (libarcstk REQUIRED )

list (APPEND REQUIRED_LIBRARIES ${libarcstk_LIBRARIES} )


## --- Configure optional features

## Default configuration for optional features

option (WITH_FLAC    "Add FLAC reading capability"     ON )
option (WITH_WAVPACK "Add WavPack reading capability"  ON )
option (WITH_FFMPEG  "Add ffmpeg reading capabilities" ON )

## libsndfile is not yet fully supported and experimental

option (WITH_LIBSNDFILE "Add libsndfile reading capabilities" OFF )

## libcdio is not yet fully supported and experimental

option (WITH_LIBCDIO    "Add libcdio reading capabilities" OFF )

## include-what-you-use is helpful when (re-)organizing headers

option (IWYU "Request use of include-what-you-use" OFF )

option (WITH_NATIVE "Use platform specific optimization on compiling" ON )


## --- Required object target: parserdev (requires libcdio)

add_library (parserdev OBJECT ${PROJECT_SOURCE_DIR}/parserdev.cpp )

find_package (libcdio 2.0.0 REQUIRED )
target_include_directories (parserdev PRIVATE ${libcdio_INCLUDE_DIRS} )

set_target_properties (parserdev PROPERTIES
	POSITION_INDEPENDENT_CODE ON
	CXX_STANDARD  14 )

if (WITH_NATIVE )

	target_compile_options (parserdev
		PUBLIC ${FLAGS_WARNINGS} ${FLAGS_OPTIMIZE} )
else ()

	target_compile_options (parserdev PUBLIC ${FLAGS_WARNINGS} )
endif (WITH_NATIVE )

list (APPEND REQUIRED_OBJECTS   $<TARGET_OBJECTS:parserdev> )
list (APPEND REQUIRED_LIBRARIES ${libcdio_LIBRARIES} )


## --- Required object target: parsercue (requires libcue)

add_library (parsercue OBJECT ${PROJECT_SOURCE_DIR}/parsercue.cpp )

find_package (libcue 2.0.0 REQUIRED )
target_include_directories (parsercue PRIVATE ${libcue_INCLUDE_DIRS} )

set_target_properties (parsercue PROPERTIES
	POSITION_INDEPENDENT_CODE ON
	CXX_STANDARD  14 )

if (WITH_NATIVE )

	target_compile_options (parsercue
		PUBLIC ${FLAGS_WARNINGS} ${FLAGS_OPTIMIZE} )
else ()

	target_compile_options (parsercue PUBLIC ${FLAGS_WARNINGS} )
endif (WITH_NATIVE )

list (APPEND REQUIRED_OBJECTS   $<TARGET_OBJECTS:parsercue> )
list (APPEND REQUIRED_LIBRARIES ${libcue_LIBRARIES} )


## --- Define optional objects and libraries to link to the project target

set (OPTIONAL_OBJECTS )

set (OPTIONAL_LIBRARIES )

## --- Optional: Add FLAC (default: ON)

if (WITH_FLAC)

	message (STATUS "Build with FLAC reading capability" )

	find_package (FLAC 1.3.1 REQUIRED ) ## checks libFLAC as well as libFLAC++

	add_library (readerflac OBJECT ${PROJECT_SOURCE_DIR}/readerflac.cpp )
	target_include_directories (readerflac PRIVATE ${FLAC_INCLUDE_DIRS} )

	## We want to link this object into a shared object
	#set_property (TARGET readerflac PROPERTY POSITION_INDEPENDENT_CODE ON )

	set_target_properties (readerflac PROPERTIES
		POSITION_INDEPENDENT_CODE ON
		CXX_STANDARD  14 )

	if (WITH_NATIVE )

		target_compile_options (readerflac
			PUBLIC ${FLAGS_WARNINGS} ${FLAGS_OPTIMIZE} )
	else ()

		target_compile_options (readerflac PUBLIC ${FLAGS_WARNINGS} )
	endif (WITH_NATIVE )

	list (APPEND OPTIONAL_OBJECTS   $<TARGET_OBJECTS:readerflac> )
	list (APPEND OPTIONAL_LIBRARIES ${FLAC_LIBRARIES} )

else (WITH_FLAC)

	message (STATUS "Build without FLAC reading capability" )
endif (WITH_FLAC)


## --- Optional: Add WavPack (default: ON)

if (WITH_WAVPACK)

	message (STATUS "Build with WavPack reading capability" )

	find_package (libwavpack 5.0.0 REQUIRED )

	add_library (readerwvpk OBJECT ${PROJECT_SOURCE_DIR}/readerwvpk.cpp )
	target_include_directories (readerwvpk PRIVATE ${libwavpack_INCLUDE_DIRS} )

	## We want to link this object into a shared object
	#set_property (TARGET readerwvpk PROPERTY POSITION_INDEPENDENT_CODE ON )

	set_target_properties (readerwvpk PROPERTIES
		POSITION_INDEPENDENT_CODE ON
		CXX_STANDARD  14 )

	if (WITH_NATIVE )

		target_compile_options (readerwvpk
			PUBLIC ${FLAGS_WARNINGS} ${FLAGS_OPTIMIZE} )
	else ()

		target_compile_options (readerwvpk PUBLIC ${FLAGS_WARNINGS} )
	endif (WITH_NATIVE )

	list (APPEND OPTIONAL_OBJECTS $<TARGET_OBJECTS:readerwvpk> )
	list (APPEND OPTIONAL_LIBRARIES ${libwavpack_LIBRARIES} )

else (WITH_WAVPACK)

	message (STATUS "Build without WavPack reading capability" )
endif (WITH_WAVPACK)


## --- Optional: Add ffmpeg (default: ON)

if (WITH_FFMPEG)

	message (STATUS "Build with ffmpeg support" )

	find_package (ffmpeg REQUIRED COMPONENTS avcodec avformat avutil )

	## find_package does not respect versions for components but ffmpeg
	## version is not accessible, so instead of REQUIREing, we check manually

	## --- Determine relevant part of ffmpeg API version

	## Note: This is currently only used for printing the API version. We have
	## to distinguish between the API from 3.1 to 4.0 and the API of 4.0 and
	## above and maybe more versions. FFmpeg provides some macros to check the
	## module versions at buildtime.

	## ffmpeg 3.1:
	## API Change: deprecated avcodec_decode_audio4() from ffmpeg 0.9 in favor
	## of avcodec_send_packet()/avcodec_receive_frame()
	set (MIN_AVCODEC_VERSION  "57.37.100" ) ## 2016-04-21
	set (MIN_AVFORMAT_VERSION "57.33.100" ) ## 2016-04-11
	set (MIN_AVUTIL_VERSION   "55.22.100" ) ## 2016-04-14

	if (${avcodec_VERSION} VERSION_LESS ${MIN_AVCODEC_VERSION} )
		message (FATAL_ERROR
		"libavcodec has version ${avcodec_VERSION} but requires at least ${MIN_AVCODEC_VERSION}")
	endif()

	if (${avformat_VERSION} VERSION_LESS ${MIN_AVFORMAT_VERSION} )
		message (FATAL_ERROR
		"libavformat has version ${avformat_VERSION} but requires at least ${MIN_AVFORMAT_VERSION}")
	endif()

	if (${avutil_VERSION} VERSION_LESS ${MIN_AVUTIL_VERSION} )
		message (FATAL_ERROR
		"libavutils has version ${avutils_VERSION} but requires at least ${MIN_AVUTIL_VERSION}")
	endif()

	## Commented out: check for ffmpeg 4.0 with API changes
	### ffmpeg 4.0:
	### API Change: deprecated av_register_all() in favor of av_muxer_iterate()
	### and av_demuxer_iterate(), registering is obsolete
	#if (${avcodec_VERSION}  VERSION_GREATER_EQUAL 58.17.100 AND ## 2018-04-11
	#	${avformat_VERSION} VERSION_GREATER_EQUAL 58.9.100  AND ## 2018-04-01
	#	${avutil_VERSION}   VERSION_GREATER_EQUAL 56.13.100 )   ## 2018-04-03
	#
	#	set (FFMPEG_API_VERSION 4.0 )
	#endif()

	add_library (readerffmpeg OBJECT ${PROJECT_SOURCE_DIR}/readerffmpeg.cpp )
	target_include_directories (readerffmpeg PRIVATE ${avcodec_INCLUDE_DIRS} )
	target_include_directories (readerffmpeg PRIVATE ${avformat_INCLUDE_DIRS} )
	target_include_directories (readerffmpeg PRIVATE ${avutil_INCLUDE_DIRS} )

	## We want to link this object into a shared object
	# set_property (TARGET readerffmpeg PROPERTY POSITION_INDEPENDENT_CODE ON )

	set_target_properties (readerffmpeg PROPERTIES
		POSITION_INDEPENDENT_CODE ON
		CXX_STANDARD  14 )

	if (WITH_NATIVE )

		target_compile_options (readerffmpeg
			PUBLIC ${FLAGS_WARNINGS} ${FLAGS_OPTIMIZE} )
	else ()

		target_compile_options (readerffmpeg PUBLIC ${FLAGS_WARNINGS} )
	endif (WITH_NATIVE )

	list (APPEND OPTIONAL_OBJECTS   $<TARGET_OBJECTS:readerffmpeg> )
	list (APPEND OPTIONAL_LIBRARIES ${avcodec_LIBRARIES} )
	list (APPEND OPTIONAL_LIBRARIES ${avformat_LIBRARIES} )
	list (APPEND OPTIONAL_LIBRARIES ${avutil_LIBRARIES} )

else (WITH_FFMPEG)

	message (STATUS "Build without ffmpeg support" )
endif (WITH_FFMPEG)


## --- Optional: Add libsndfile (default: OFF)

if (WITH_LIBSNDFILE)

	message (STATUS "Build with libsndfile support" )

	find_package (libsndfile 1.0.17 REQUIRED )

	add_library (readersndfile OBJECT ${PROJECT_SOURCE_DIR}/readersndfile.cpp )
	target_include_directories (readersndfile
		PRIVATE ${libsndfile_INCLUDE_DIRS})

	## We want to link this object into a shared object
	#set_property (TARGET readersndfile PROPERTY POSITION_INDEPENDENT_CODE ON )

	set_target_properties (readersndfile PROPERTIES
		POSITION_INDEPENDENT_CODE ON
		CXX_STANDARD  14 )

	if (WITH_NATIVE )

		target_compile_options (readersndfile
			PUBLIC ${FLAGS_WARNINGS} ${FLAGS_OPTIMIZE} )
	else ()

		target_compile_options (readersndfile PUBLIC ${FLAGS_WARNINGS} )
	endif (WITH_NATIVE )

	list (APPEND OPTIONAL_OBJECTS   $<TARGET_OBJECTS:readersndfile> )
	list (APPEND OPTIONAL_LIBRARIES ${libsndfile_LIBRARIES} )

else (WITH_LIBSNDFILE)

	message (STATUS "Build without libsndfile support" )
endif (WITH_LIBSNDFILE)


## --- Optional: Add include-what-you-use support (default: OFF)

if (IWYU)

	message (STATUS "Program include-what-you-use requested" )
	find_program (IWYU_PATH NAMES include-what-you-use iwyu )

	if (IWYU_PATH)

		## Set for all subsequent targets
		set (CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PATH} )

		## Commented out: Set for specific targets only
		#set_property(TARGET ${TARGET_NAME}
		#	PROPERTY CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PATH})

		message (STATUS "Program include-what-you-use found: ${IWYU_PATH}" )

	else (IWYU_PATH)

		message (WARNING "Program include-what-you-use not found" )

	endif (IWYU_PATH)
endif (IWYU)


## --- Configured files by CMake

## Configure all .in files producing C++ headers files

set (CONFIGURED_HEADERS
	audioformats.hpp
	metaformats.hpp
	version.hpp )

foreach (_header IN LISTS CONFIGURED_HEADERS )

	configure_file (
	  "${PROJECT_SOURCE_DIR}/${_header}.in"
	  "${PROJECT_BINARY_DIR}/${_header}"
	  @ONLY
	)
endforeach()

## Configure .pc file for pkg-info

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/libarcsdec.pc.in"
  "${PROJECT_BINARY_DIR}/libarcsdec.pc"
  @ONLY
)


## --- Specify Exported headers

set (EXPORTED_HEADERS )

list (APPEND EXPORTED_HEADERS "${PROJECT_SOURCE_DIR}/audioreader.hpp" )
list (APPEND EXPORTED_HEADERS "${PROJECT_SOURCE_DIR}/calculators.hpp" )
list (APPEND EXPORTED_HEADERS "${PROJECT_SOURCE_DIR}/descriptors.hpp" )
list (APPEND EXPORTED_HEADERS "${PROJECT_SOURCE_DIR}/metaparser.hpp" )
list (APPEND EXPORTED_HEADERS "${PROJECT_SOURCE_DIR}/sampleproc.hpp" )


## --- Add and configure project target

## Build libarcsdec shared library as project target

add_library (${PROJECT_NAME} SHARED
	${SOURCES}
	${REQUIRED_OBJECTS}
	${OPTIONAL_OBJECTS}
)

## Include location for generated sources to search path
## (required to find version.hpp and the other configured headers)

target_include_directories (${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR} )

## Link required and optional libs to project target

target_link_libraries (${PROJECT_NAME} PRIVATE ${REQUIRED_LIBRARIES} )
target_link_libraries (${PROJECT_NAME} PRIVATE ${OPTIONAL_LIBRARIES} )

## Configure properties for project target

set_target_properties (${PROJECT_NAME} PROPERTIES
	CXX_STANDARD  14
	PUBLIC_HEADER "${EXPORTED_HEADERS}"
	VERSION       "${PROJECT_VERSION_STRING_WITHOUT_SUFFIX}"
	SOVERSION     "${PROJECT_VERSION_STRING_WITHOUT_SUFFIX}" )


## --- Install binary and exported header files

if (NOT SKIP_INSTALL_ALL)

	install (TARGETS ${PROJECT_NAME}
		LIBRARY        DESTINATION  "${INSTALL_LIBRARY_DIR}"
		PUBLIC_HEADER  DESTINATION  "${INSTALL_INCLUDE_DIR}/arcs" )

	if (NOT SKIP_INSTALL_FILES)

		install (FILES "${PROJECT_BINARY_DIR}/libarcsdec.pc"
			DESTINATION "${INSTALL_PKGCONFIG_DIR}" )
	endif()

endif (NOT SKIP_INSTALL_ALL)


## --- Compiler Specific Settings

## Settings for g++ and clang

if (CMAKE_COMPILER_IS_GNUCXX
		OR CMAKE_CXX_COMPILER MATCHES ".*clang"
		OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )

	## Activate Warnings

	target_compile_options (${PROJECT_NAME}
		PUBLIC -Wall -Wextra -Wold-style-cast -Wpedantic -Weffc++ )

	## Platform specific optimizations (default ON for Release, otherwise OFF)

	if (CMAKE_BUILD_TYPE STREQUAL "Release" )

		set (WITH_NATIVE ON CACHE BOOL
			"Build with platform specific optimization" FORCE )

	endif (CMAKE_BUILD_TYPE STREQUAL "Release" )

	if (WITH_NATIVE )

		message (STATUS "Build with platform specific optimization" )

		target_compile_options (${PROJECT_NAME}
			PUBLIC -march=native -mtune=generic )
	else()

		message (STATUS "Build without platform specific optimization" )

	endif (WITH_NATIVE )

else()

	message (WARNING "No compile flags specified for the actual C++ compiler" )
	message (WARNING "Try a generic compile without project specific flags" )

endif()


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
option (WITH_IMPLONLY_DOCS "Build documentation also for implementations" OFF )
option (USE_MCSS           "Build documentation using m.css"              OFF )

## Auto-activate WITH_DOCS if settings for documentation are present
if (WITH_INTERNAL_DOCS OR WITH_IMPLONLY_DOCS OR USE_MCSS )

	set(WITH_DOCS ON )

endif()

## Process configuration for building the documentation
if (WITH_DOCS )

	add_subdirectory(${PROJECT_DOC_DIR} )

endif (WITH_DOCS )


## --- Cleanup

## Unset all cached option values to have their default values active
## on the next invokation of CMake

unset (IWYU               CACHE)
unset (WITH_DOCS          CACHE)
unset (WITH_INTERNAL_DOCS CACHE)
unset (WITH_IMPLONLY_DOCS CACHE)
unset (WITH_NATIVE        CACHE)
unset (WITH_TESTS         CACHE)
unset (USE_MCSS           CACHE)
unset (WITH_FLAC          CACHE)
unset (WITH_WAVPACK       CACHE)
unset (WITH_FFMPEG        CACHE)
unset (WITH_LIBSNDFILE    CACHE)

