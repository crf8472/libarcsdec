## libarcsdec: Define project directory layout and paths
## vim:fdm=marker

## INSTALL TREE

## Subdir for installed includes
set (LIBARCSDEC_INCLUDE_INSTALL_SUBDIR "include/arcsdec" )


## BUILD TREE

## Root directory for out-of-source build
set (LIBARCSDEC_BINARY_DIR         "${CMAKE_CURRENT_BINARY_DIR}" )

## Target directory for non-cmake logs
set (LIBARCSDEC_LOG_BINARY_DIR     "${LIBARCSDEC_BINARY_DIR}/log" )

## Target directory for generated sources and configuration
set (LIBARCSDEC_GENSRC_BINARY_DIR  "${LIBARCSDEC_BINARY_DIR}/generated-sources" )

## Target directory for public/exported headers
set (LIBARCSDEC_INCLUDE_BINARY_DIR
	"${LIBARCSDEC_BINARY_DIR}/${LIBARCSDEC_INCLUDE_INSTALL_SUBDIR}" )


## SOURCE TREE

## Project root directory
set (LIBARCSDEC_ROOT_DIR           "${CMAKE_CURRENT_SOURCE_DIR}" )

## Root directory for sources
set (LIBARCSDEC_SOURCE_DIR         "${LIBARCSDEC_ROOT_DIR}/src" )

## Root directory for public/exported headers
set (LIBARCSDEC_INCLUDE_SOURCE_DIR "${LIBARCSDEC_ROOT_DIR}/include" )


## Exported/Public Headers

list (APPEND LIBARCSDEC_PUBLIC_HEADERS
	"${LIBARCSDEC_INCLUDE_SOURCE_DIR}/audioreader.hpp"
	"${LIBARCSDEC_INCLUDE_SOURCE_DIR}/calculators.hpp"
	"${LIBARCSDEC_INCLUDE_SOURCE_DIR}/descriptor.hpp"
	"${LIBARCSDEC_INCLUDE_SOURCE_DIR}/metaparser.hpp"
	"${LIBARCSDEC_INCLUDE_SOURCE_DIR}/sampleproc.hpp"
	"${LIBARCSDEC_INCLUDE_SOURCE_DIR}/selection.hpp"
	"${LIBARCSDEC_INCLUDE_SOURCE_DIR}/version.hpp"
)

## Define library sources

list (APPEND LIBARCSDEC_SOURCES
	# public
	"${LIBARCSDEC_SOURCE_DIR}/audioreader.cpp"
	"${LIBARCSDEC_SOURCE_DIR}/calculators.cpp"
	"${LIBARCSDEC_SOURCE_DIR}/descriptor.cpp"
	"${LIBARCSDEC_SOURCE_DIR}/metaparser.cpp"
	"${LIBARCSDEC_SOURCE_DIR}/sampleproc.cpp"
	"${LIBARCSDEC_SOURCE_DIR}/selection.cpp"
	# internal
	"${LIBARCSDEC_SOURCE_DIR}/flexbisondriver.cpp"
	"${LIBARCSDEC_SOURCE_DIR}/libinspect.cpp"
	"${LIBARCSDEC_SOURCE_DIR}/tochandler.cpp"
)

