## CMake file for libwavepack based Wavpack audio reader
## vim:fdm=marker

cmake_minimum_required (VERSION 3.14.0 )

message (STATUS "Build with WavPack reading capability for wavpack audio" )

function (readerwvpk_pkg DEPS )
	set  (${DEPS} "wavpack >= 5.0.0" PARENT_SCOPE )
endfunction()

find_package (libwavpack 5.0.0 REQUIRED )

target_include_directories (${PROJECT_NAME} PRIVATE ${libwavpack_INCLUDE_DIRS} )
target_link_libraries      (${PROJECT_NAME} PUBLIC  ${libwavpack_LIBRARIES} )
target_sources             (${PROJECT_NAME} PRIVATE readerwvpk.cpp )

