## CMake file for libcue based Cuesheet parser
## vim:fdm=marker

## - Variables: PKG_REQUIRE_ARRAY, PROJECT_NAME, PROJECT_SOURCE_DIR

cmake_minimum_required (VERSION 3.14.0 )


message (STATUS "Build with libcue parsing capability for cuesheets" )

find_package (libcue 2.0.0 REQUIRED )
list (APPEND PKG_REQUIRE_ARRAY "libcue >= 2.0.0" )

target_include_directories (${PROJECT_NAME} PRIVATE ${libcue_INCLUDE_DIRS} )

target_link_libraries (${PROJECT_NAME} PRIVATE ${libcue_LIBRARIES} )

target_sources (${PROJECT_NAME} PRIVATE parserlibcue.cpp )

