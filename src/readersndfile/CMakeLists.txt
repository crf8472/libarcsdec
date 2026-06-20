## CMake file for libsndfile based audio reader
## vim:fdm=marker

cmake_minimum_required (VERSION 3.14.0 )

message (STATUS "Build with libsndfile support for audio" )

function (readersndfile_pkg DEPS )
	set  (${DEPS} "sndfile >= 1.0.17" PARENT_SCOPE )
endfunction()

find_package (SndFile 1.0.17 REQUIRED )

if (TARGET SndFile::sndfile )
	## found by libsndfile's cmake config
	target_link_libraries (${PROJECT_NAME} PRIVATE SndFile::sndfile )
else()
	## found by cmake/Modules/Findlibsndfile.cmake

	if (NOT libsndfile_LIBRARIES )
		find_package (libsndfile 1.0.17 REQUIRED )
	endif()

	target_include_directories (${PROJECT_NAME}
		PRIVATE ${libsndfile_INCLUDE_DIRS} )

	target_link_libraries (${PROJECT_NAME}
		PUBLIC ${libsndfile_LIBRARIES} )
endif()

target_sources (${PROJECT_NAME} PRIVATE readersndfile.cpp )

