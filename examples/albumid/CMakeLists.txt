## Build example application 'albumid', part of libarcsdec
## vim: fdm=marker

cmake_minimum_required (VERSION 3.10 )

add_executable (libarcsdec_albumid EXCLUDE_FROM_ALL albumid.cpp )

set_target_properties ( libarcsdec_albumid PROPERTIES
	CXX_STANDARD          17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS        OFF
)

target_compile_options ( libarcsdec_albumid
	PRIVATE -Wall -Wextra -pedantic )

add_dependencies ( libarcsdec_albumid libarcsdec )


## build

## Always include the headers in the build/ dir - if another header is present,
## ignore it.
target_include_directories ( libarcsdec_albumid
	PRIVATE $<BUILD_INTERFACE:${LIBARCSDEC_BINARY_DIR}/libs/libarcstk/include>
	PRIVATE $<BUILD_INTERFACE:${LIBARCSDEC_BINARY_DIR}/include> )

## Always link to the binaries in the build/ dir - if another binary is present,
## ignore it.
target_link_libraries ( libarcsdec_albumid
	PRIVATE "-Wl,--disable-new-dtags,-rpath,${LIBARCSDEC_BINARY_DIR}/libs/" libarcstk
	PRIVATE "-Wl,--disable-new-dtags,-rpath,${LIBARCSDEC_BINARY_DIR}/"      libarcsdec )

