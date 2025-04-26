## CMake file for ffmpeg based audio reader
## vim:fdm=marker

## - Variables: PKG_REQUIRE_ARRAY, PROJECT_NAME, PROJECT_SOURCE_DIR

cmake_minimum_required (VERSION 3.14.0 )


message (STATUS "Build with ffmpeg support for audio" )

## find_package does not respect versions for components but the ffmpeg
## version is not trivial to access in a Find module.
## Thus, instead of REQUIREing the lib versions, we verify them manually.

find_package (ffmpeg REQUIRED COMPONENTS avcodec avformat avutil )

## Verify the required versions of ffmpeg libs
macro (at_least_version _required _actual )

	if (${_actual} VERSION_LESS ${_required})

		message (FATAL_ERROR
		"Actual version is ${_actual}, required is at least ${_required}." )

	endif()

endmacro()

## ffmpeg 3.1 < x < 4.0:
## API Change: deprecated avcodec_decode_audio4() from ffmpeg 0.9 in favor
## of avcodec_send_packet()/avcodec_receive_frame()
at_least_version ("57.37.100" ${avcodec_VERSION}  ) ## 2016-04-21
at_least_version ("57.33.100" ${avformat_VERSION} ) ## 2016-04-11
at_least_version ("55.22.100" ${avutil_VERSION}   ) ## 2016-04-14
list (APPEND PKG_REQUIRE_ARRAY "libavcodec >= 57.37.100" )
list (APPEND PKG_REQUIRE_ARRAY "libavformat >= 57.33.100" )
list (APPEND PKG_REQUIRE_ARRAY "libavutil >= 55.22.100" )

## Commented out, API change is small, can be distinguished in source.
## ffmpeg x >= 4.0:
## API Change: deprecated av_register_all() in favor of av_muxer_iterate()
## and av_demuxer_iterate(), registering is obsolete
#at_least_version ("58.17.100" ${avcodec_VERSION} )  ## 2018-04-11
#at_least_version ("58.9.100"  ${avformat_VERSION} ) ## 2018-04-01
#at_least_version ("56.13.100" ${avutil_VERSION} )   ## 2018-04-03

## TODO
## ffmpeg x >= 5.1:
## AVFrame::channels, AVFrame::channel_layout is deprecated in favor of
## AVFrame::ch_layout (nb_channels and ch_layout).
## AVCodecContext::channels, AVCodecContext::channel_layout is deprecated
## in favor of AVCodecContext::ch_layout (nb_channels and ch_layout).
#at_least_version ("59.24.100" ${avcodec_VERSION} )  ## 2022-03-15
#at_least_version ("59.19.100" ${avformat_VERSION} ) ## 2022-03-15
#at_least_version ("57.24.100" ${avutil_VERSION} )   ## 2022-03-15

target_include_directories (${PROJECT_NAME}
	PRIVATE ${avcodec_INCLUDE_DIRS}
	PRIVATE ${avformat_INCLUDE_DIRS}
	PRIVATE ${avutil_INCLUDE_DIRS} )

target_link_libraries (${PROJECT_NAME}
	PUBLIC ${avcodec_LIBRARIES}
	PUBLIC ${avformat_LIBRARIES}
	PUBLIC ${avutil_LIBRARIES} )

target_sources (${PROJECT_NAME} PRIVATE readerffmpeg.cpp )

