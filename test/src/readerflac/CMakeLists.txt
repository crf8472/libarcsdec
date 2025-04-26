## CMake build script for testcases of readerflac.cpp.
## vim:fdm=marker

cmake_minimum_required (VERSION 3.10 )
## --- Policies {{{1

## Add link library FLAC::FLAC++ to target that is not built in this directory.
cmake_policy (SET CMP0079 NEW )

## 1}}}

message (STATUS "Build testcases for fLaC audio reader" )

## FIXME Find a way to avoid doing find_package() again
find_package (Threads       REQUIRED )
find_package (FLAC    1.3.1 REQUIRED ) ## checks libFLAC and libFLAC++

## --- Add flac dependencies to testcase 'readerflac_details' {{{1

if (TARGET FLAC::FLAC++ )

	target_link_libraries (readerflac_details_test PRIVATE FLAC::FLAC++ )

else()

	target_include_directories (readerflac_details_test
		PRIVATE ${LIBFLACPP_INCLUDE_DIRS} )

	target_link_libraries (readerflac_details_test
		PRIVATE ${LIBFLACPP_LIBRARIES} )
endif()

