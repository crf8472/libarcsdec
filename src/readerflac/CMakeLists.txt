## CMake file for libflac based FLAC audio reader
## vim:fdm=marker

cmake_minimum_required (VERSION 3.14.0 )

message (STATUS "Build with fLaC reading capability for fLaC audio" )

function (readerflac_pkg DEPS )
	set  (${DEPS} "flac++ >= 1.3.1" PARENT_SCOPE )
endfunction()

find_package (Threads       REQUIRED )
find_package (FLAC    1.3.1 REQUIRED ) ## checks libFLAC and libFLAC++


add_library (libarcsdec_readerflac OBJECT readerflac.cpp )

set_property (TARGET libarcsdec_readerflac
	PROPERTY POSITION_INDEPENDENT_CODE ON )

target_include_directories (libarcsdec_readerflac
	PUBLIC $<BUILD_INTERFACE:${LIBARCSDEC_INCLUDE_SOURCE_DIR}>## public headers
	PUBLIC $<BUILD_INTERFACE:${LIBARCSDEC_SOURCE_DIR}>        ## private headers
)

target_link_libraries      (libarcsdec_readerflac
	PUBLIC libarcstk::libarcstk # TODO Should be propagated by main target
)

if (TARGET FLAC::FLAC++ )
	## found by flac's cmake config
	target_link_libraries (libarcsdec_readerflac
		PUBLIC FLAC::FLAC++
		PUBLIC FLAC::FLAC
	)
else()
	## found by cmake/Modules/FindFLAC
	target_include_directories (libarcsdec_readerflac
		PUBLIC $<BUILD_INTERFACE:${LIBFLACPP_INCLUDE_DIRS}>
	)

	target_link_libraries (libarcsdec_readerflac
		PUBLIC ${LIBFLACPP_LIBRARIES}
		PUBLIC ${LIBFLAC_LIBRARIES}
	)
endif()

## Put this into libarcsdec
target_link_libraries (${PROJECT_NAME} PRIVATE libarcsdec_readerflac )

