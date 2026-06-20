## CMake file for libcue based Cuesheet parser
## vim:fdm=marker

cmake_minimum_required (VERSION 3.14.0 )

message (STATUS "Build with libcue parsing capability for cuesheets" )

function (parserlibcue_pkg DEPS )
	set  (${DEPS} "libcue >= 2.0.0" PARENT_SCOPE )
endfunction()

find_package (libcue 2.0.0 REQUIRED )

target_include_directories (${PROJECT_NAME} PRIVATE ${libcue_INCLUDE_DIRS} )
target_link_libraries      (${PROJECT_NAME} PRIVATE ${libcue_LIBRARIES} )
target_sources             (${PROJECT_NAME} PRIVATE parserlibcue.cpp )

