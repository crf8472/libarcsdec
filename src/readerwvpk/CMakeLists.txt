## CMake file for libwavepack based Wavpack audio reader
## vim:fdm=marker

cmake_minimum_required (VERSION 3.14.0 )

message (STATUS "Build with WavPack reading capability for wavpack audio" )

function (readerwvpk_pkg DEPS )
	set  (${DEPS} "wavpack >= 5.0.0" PARENT_SCOPE )
endfunction()

find_package (libwavpack 5.0.0 REQUIRED )

#target_sources             (${PROJECT_NAME} PRIVATE readerwvpk.cpp )


add_library (libarcsdec_readerwvpk OBJECT readerwvpk.cpp )

set_property (TARGET libarcsdec_readerwvpk
	PROPERTY POSITION_INDEPENDENT_CODE ON )

target_include_directories (libarcsdec_readerwvpk
	PUBLIC $<BUILD_INTERFACE:${LIBARCSDEC_INCLUDE_SOURCE_DIR}>## public headers
	PUBLIC $<BUILD_INTERFACE:${LIBARCSDEC_SOURCE_DIR}>        ## private headers
	PUBLIC $<BUILD_INTERFACE:${libwavpack_INCLUDE_DIRS}>
)

target_link_libraries      (libarcsdec_readerwvpk
	PUBLIC libarcstk::libarcstk # TODO Should be propagated by main target
	PUBLIC ${libwavpack_LIBRARIES}
)

## Put this into libarcsdec
target_link_libraries (${PROJECT_NAME} PRIVATE libarcsdec_readerwvpk )

