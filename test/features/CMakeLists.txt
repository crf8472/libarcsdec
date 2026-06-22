## CMake build script for adding all subdirectories
## vim:fdm=marker

cmake_minimum_required (VERSION 3.10 )


## parsers

add_subdirectory (parsercue )
add_subdirectory (parsertoc )

if (WITH_LIBCUE )
	add_subdirectory (parserlibcue )
else ()
	message (STATUS "Skip tests for parserlibcue due to WITH_LIBCUE=OFF" )
endif()


## readers

add_subdirectory (readerwav )

if (WITH_FFMPEG )
	add_subdirectory (readerffmpeg  )
else ()
	message (STATUS "Skip tests for readerffmpeg due to WITH_FFMPEG=OFF" )
endif ()

if (WITH_FLAC )
	add_subdirectory (readerflac    )
else ()
	message (STATUS "Skip tests for readerflac due to WITH_FLAC=OFF" )
endif ()

if (WITH_LIBSNDFILE )
	add_subdirectory (readersndfile )
else ()
	message (STATUS "Skip tests for readersndfile due to WITH_LIBSNDFILE=OFF" )
endif()

if (WITH_WAVPACK )
	add_subdirectory (readerwvpk    )
else ()
	message (STATUS "Skip tests for readerwvpk due to WITH_WAVPACK=OFF" )
endif()

