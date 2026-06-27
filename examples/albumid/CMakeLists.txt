## Build example application 'albumid', part of libarcsdec
## vim: fdm=marker

cmake_minimum_required (VERSION 3.10 )

add_executable (libarcsdec_albumid EXCLUDE_FROM_ALL albumid.cpp )
add_dependencies (libarcsdec_examples libarcsdec_albumid )

set_target_properties (libarcsdec_albumid PROPERTIES
	CXX_STANDARD          17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS        OFF
	OUTPUT_NAME           albumid
	BUILD_RPATH           "${LIBARCSDEC_BINARY_DIR}"
	BUILD_WITH_INSTALL_RPATH    FALSE
	INSTALL_RPATH_USE_LINK_PATH TRUE
)

target_compile_options (libarcsdec_albumid
	PRIVATE -Wall -Wextra -pedantic )

add_dependencies (libarcsdec_albumid libarcsdec )


## build

## Always include the headers in the build/ dir - if another header is present,
## ignore it.
target_include_directories (libarcsdec_albumid
	PRIVATE $<BUILD_INTERFACE:${LIBARCSDEC_BINARY_DIR}/libs/libarcstk/include>
	PRIVATE $<BUILD_INTERFACE:${LIBARCSDEC_BINARY_DIR}/include> )

## Always link to the binaries in the build/ dir - if another binary is present,
## ignore it.
target_link_libraries (libarcsdec_albumid
	PRIVATE libarcstk
	PRIVATE libarcsdec )

