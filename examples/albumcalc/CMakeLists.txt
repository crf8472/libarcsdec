## Build example application 'albumcalc', part of libarcsdec
## vim: fdm=marker

cmake_minimum_required (VERSION 3.10 )

add_executable (libarcsdec_albumcalc EXCLUDE_FROM_ALL albumcalc.cpp )
add_dependencies (libarcsdec_examples libarcsdec_albumcalc )

set_target_properties (libarcsdec_albumcalc PROPERTIES
	CXX_STANDARD          17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS        OFF
	OUTPUT_NAME           albumcalc
	BUILD_RPATH           "${LIBARCSDEC_BINARY_DIR}"
	BUILD_WITH_INSTALL_RPATH    FALSE
	INSTALL_RPATH_USE_LINK_PATH TRUE
)

target_compile_options (libarcsdec_albumcalc
	PRIVATE -Wall -Wextra -pedantic )

add_dependencies (libarcsdec_albumcalc libarcsdec )


## build

## Always include the headers in the build/ dir - if another header is present,
## ignore it.
target_include_directories (libarcsdec_albumcalc
	PRIVATE $<BUILD_INTERFACE:${LIBARCSDEC_BINARY_DIR}/libs/libarcstk/include>
	PRIVATE $<BUILD_INTERFACE:${LIBARCSDEC_BINARY_DIR}/include> )

## Always link to the binaries in the build/ dir - if another binary is present,
## ignore it.
target_link_libraries (libarcsdec_albumcalc
	PRIVATE libarcstk
	PRIVATE libarcsdec )

