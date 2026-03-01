## Build example application 'albumid', part of libarcsdec
## vim: fdm=marker

add_executable ( albumid albumid.cpp )

set_target_properties ( albumid
	PROPERTIES
	CXX_STANDARD          17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS        OFF )

target_compile_options ( albumid PRIVATE -Wall -Wextra -pedantic )

add_dependencies ( albumid libarcsdec )


## build

set (PROJECT_BUILD_DIR "${PROJECT_ROOT_DIR}/build" )

## Always include the headers in the build/ dir - if another header is present,
## ignore it.
target_include_directories ( albumid
	PRIVATE $<BUILD_INTERFACE:${PROJECT_BUILD_DIR}/libs/libarcstk/include>
	PRIVATE $<BUILD_INTERFACE:${PROJECT_BUILD_DIR}/include> )

## Always link to the binaries in the build/ dir - if another binary is present,
## ignore it.
target_link_libraries ( albumid
	PRIVATE "-Wl,--disable-new-dtags,-rpath,${PROJECT_BUILD_DIR}/libs/" libarcstk
	PRIVATE "-Wl,--disable-new-dtags,-rpath,${PROJECT_BUILD_DIR}/" libarcsdec )

