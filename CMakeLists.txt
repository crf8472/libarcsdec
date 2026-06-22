## Root CMake file for libarcsdec
## vim:fdm=marker

cmake_minimum_required (VERSION 3.15.0 )

project (libarcsdec VERSION 0.3.0 LANGUAGES CXX )

set (LIBARCSDEC_VERSION_SUFFIX "alpha.1" )
set (LIBARCSDEC_VERSION_STRING "${PROJECT_VERSION}-${LIBARCSDEC_VERSION_SUFFIX}" )

set (PROJECT_DESCRIPTION "Audio decoder and TOC parser adapters for libarcstk" )

set (PROJECT_HOMEPAGE_URL "https://github.com/crf8472/${PROJECT_NAME}" )

message (STATUS "Building ${PROJECT_NAME} ${LIBARCSDEC_VERSION_STRING}" )

## --- Includes {{{1
set (_libarcsdec_cmake_functions_dir
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/functions" )

## Mandatory configuration
include ("${_libarcsdec_cmake_functions_dir}/ProjectFilesAndDirectories.cmake" )
include ("${_libarcsdec_cmake_functions_dir}/BuildTreeSetup.cmake" )
include ("${_libarcsdec_cmake_functions_dir}/CompilerFlags.cmake" )
include ("${_libarcsdec_cmake_functions_dir}/GitInfo.cmake" )
include ("${_libarcsdec_cmake_functions_dir}/Dependencies.cmake" )

## Optional support for development
include ("${_libarcsdec_cmake_functions_dir}/ToolClangTidy.cmake" )
include ("${_libarcsdec_cmake_functions_dir}/ToolClangIncludeGraph.cmake" )
include ("${_libarcsdec_cmake_functions_dir}/ToolCodeCoverage.cmake" )
include ("${_libarcsdec_cmake_functions_dir}/ToolGraphViz.cmake" )

unset (_libarcsdec_cmake_functions_dir )
## 1}}}
## --- CMake Setup {{{1

set (CMAKE_EXPORT_COMPILE_COMMANDS ON ) ## required for clang-tidy and iwyu

## --- Setup Default Build Type {{{2

if (NOT CMAKE_BUILD_TYPE )

	set (CMAKE_BUILD_TYPE "Release"
		CACHE STRING "Choose the type of build (e.g. Debug or Release)" FORCE )

	message (STATUS "No build type specified, set to ${CMAKE_BUILD_TYPE}" )
endif (NOT CMAKE_BUILD_TYPE )

message (STATUS "Build type: ${CMAKE_BUILD_TYPE}" )
## 2}}}
## --- Modules {{{2

## Tell find_package to first try locating libarcstk via system-installed
## .config file instead of project's FindLibarcstk.cmake module
set (CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE ) ## since CMake 3.15

## Search path for cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

# 2}}}
## 1}}}

## --- Setup Build Tree {{{1
libarcsdec_setup_build_tree()

# 1}}}
## --- Trigger Configuration Step Whenever Git Index Changes {{{1

set_property (DIRECTORY APPEND
	PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/.git/index" )

libarcsdec_git_get_version_string (LIBARCSDEC_GIT_VERSION_STRING )

configure_file ("${LIBARCSDEC_SOURCE_DIR}/version.cpp.in"
	"${LIBARCSDEC_GENSRC_BINARY_DIR}/version.cpp"
	@ONLY )

# 1}}}
## --- Define Project Target {{{1

add_library (${PROJECT_NAME} SHARED
	${LIBARCSDEC_SOURCES}
	"${LIBARCSDEC_GENSRC_BINARY_DIR}/version.cpp"
)

get_directory_property (HAS_PARENT PARENT_DIRECTORY )

if (HAS_PARENT )
	## Make project's includes available to PARENT
	target_include_directories (${PROJECT_NAME}
		## public headers:
		INTERFACE $<BUILD_INTERFACE:${LIBARCSDEC_BINARY_DIR}/include>
	)

	## Provide a namespaced target also when added by add_subdirectory
	add_library (${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME} )
endif()

target_include_directories (${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${LIBARCSDEC_INCLUDE_BINARY_DIR}>
		$<INSTALL_INTERFACE:${LIBARCSDEC_INCLUDE_INSTALL_SUBDIR}>
	PRIVATE
		$<BUILD_INTERFACE:${LIBARCSDEC_SOURCE_DIR}> ## private headers
		$<BUILD_INTERFACE:${LIBARCSDEC_GENSRC_BINARY_DIR}> ## generated sources
	)

set_target_properties (${PROJECT_NAME} PROPERTIES
	CXX_STANDARD   17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF ## use -std=c++17 instead of std=gnu++17
	PUBLIC_HEADER  "${LIBARCSDEC_PUBLIC_HEADERS}"
	PREFIX         "" ## No 'lib' prefix in soname, PROJECT_NAME is prefixed
	VERSION        "${PROJECT_VERSION}"
	SOVERSION      "${PROJECT_VERSION}" )

add_dependencies (${PROJECT_NAME} libarcsdec_link_to_headers )

## 1}}}
## --- Compiler Specific Settings {{{1

libarcsdec_apply_compiler_flags (${PROJECT_NAME} )

## 1}}}

get_target_property (LIBARCSDEC_CXX_FLAGS ${PROJECT_NAME} COMPILE_OPTIONS )
message (STATUS "Compile flags: ${LIBARCSDEC_CXX_FLAGS}" )

## --- Installation {{{1

if (NOT SKIP_INSTALL_ALL )

	add_subdirectory (install ) ## relies on LIBARCSDEC_PKG_REQUIRE_ARRAY
endif ()

## 1}}}
## --- Option: WITH_TESTS {{{1

option (WITH_TESTS "Build unit tests" OFF )

if (WITH_TESTS )

	message (STATUS "Build with tests" )

	## idempotent (does not matter whether top-level or submodule)
	enable_testing()

	## Add Catch2 test framework if it is not already present
	## (i.e. if PARENT did not include it)
	if (NOT TARGET Catch2::Catch2 )
		add_subdirectory (thirdparty/catch2 EXCLUDE_FROM_ALL )
	else()
		message (WARNING "Catch2 skipped since target already exists" )
	endif()

	## Add tests
	add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/test" )

endif (WITH_TESTS )

## 1}}}
## --- Options: Build documentation {{{1

option (WITH_DOCS          "Build documentation for public API"           OFF )
option (USE_DOC_TOOL       "Build documentation using specified tool"     OFF )
option (WITH_INTERNAL_DOCS "Build documentation also for internal APIs"   OFF )
option (WITH_EXAMPLES      "Build examples"                               OFF )

## -- Process configuration for building the documentation
if (WITH_DOCS )

	add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/doc" )
else ()

	if (USE_DOC_TOOL )
		message (FATAL_ERROR "-DWITH_DOCS=ON is required to set USE_DOC_TOOL." )
	endif()
endif (WITH_DOCS )


## -- Require WITH_DOCS to use WITH_INTERNAL_DOCS
if (WITH_INTERNAL_DOCS )

#	set (WITH_DOCS ON CACHE BOOL "Build documentation for public API" FORCE )
	if (NOT WITH_DOCS )
		message (WARNING
			"-DWITH_DOCS=ON is OFF but -DWITH_INTERNAL_DOCS=ON will only "
			"have any effects as soon as -DWITH_DOCS is set ON." )
	endif()
endif()

## 1}}}
## --- Setup dependencies {{{1

option (WITH_SUBMODULES "Compile libarcstk as a submodule" OFF )

set (LIBARCSDEC_PKG_REQUIRE_ARRAY )

libarcsdec_setup_required_dependencies   (LIBARCSDEC_PKG_REQUIRE_ARRAY )
libarcsdec_setup_internal_dependencies   ()
libarcsdec_setup_configured_dependencies (LIBARCSDEC_PKG_REQUIRE_ARRAY )

# 1}}}
## --- Setup examples {{{1

if (WITH_EXAMPLES )
	add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/examples" )
endif()

## 1}}}
## --- Setup development tools {{{1
libarcsdec_enable_clang_tidy (LIBARCSDEC_CLANG_TIDY_ENABLED )
libarcsdec_enable_clang_include_graph (LIBARCSDEC_CLANG_INCLUDE_GRAPH_ENABLED )
libarcsdec_enable_code_coverage (LIBARCSDEC_CODE_COVERAGE_ENABLED )
libarcsdec_enable_target_dep_graph_by_dot (LIBARCSDEC_TARGET_DEPS_GRAPH_ENABLED )
## 1}}}

