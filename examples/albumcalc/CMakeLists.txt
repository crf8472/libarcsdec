## Build example application 'albumcalc', part of libarcsdec
## vim: fdm=marker

add_executable ( albumcalc albumcalc.cpp )

set_target_properties ( albumcalc
	PROPERTIES
	CXX_STANDARD          17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS        OFF )

target_compile_options ( albumcalc PRIVATE -Wall -Wextra -pedantic )

add_dependencies ( albumcalc libarcsdec )


## build

set (PROJECT_BUILD_DIR "${PROJECT_ROOT_DIR}/build" )

## Always include the headers in the build/ dir - if another header is present,
## ignore it.
target_include_directories ( albumcalc
	PRIVATE $<BUILD_INTERFACE:${PROJECT_BUILD_DIR}/libs/libarcstk/include>
	PRIVATE $<BUILD_INTERFACE:${PROJECT_BUILD_DIR}/include> )

## Always link to the binaries in the build/ dir - if another binary is present,
## ignore it.
target_link_libraries ( albumcalc
	PRIVATE "-Wl,--disable-new-dtags,-rpath,${PROJECT_BUILD_DIR}/libs/" libarcstk
	PRIVATE "-Wl,--disable-new-dtags,-rpath,${PROJECT_BUILD_DIR}/" libarcsdec )

