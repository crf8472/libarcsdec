## CMake build script for configuring and building feature tests
## vim:fdm=marker

add_test_suite (libarcsdec_readerffmpeg
	LABEL "readerffmpeg"
	LINK_TARGET libarcsdec_readerffmpeg
	TIMEOUT 10 )

## FIXME redundant
# find_package (ffmpeg REQUIRED COMPONENTS avcodec avformat avutil )
#
# target_include_directories (libarcsdec_readerffmpeg_tests
# 	PRIVATE ${avcodec_INCLUDE_DIRS}
# 	PRIVATE ${avformat_INCLUDE_DIRS}
# 	PRIVATE ${avutil_INCLUDE_DIRS} )
#
# target_link_libraries (libarcsdec_readerffmpeg_tests
# 	PUBLIC ${avcodec_LIBRARIES}
# 	PUBLIC ${avformat_LIBRARIES}
# 	PUBLIC ${avutil_LIBRARIES} )

