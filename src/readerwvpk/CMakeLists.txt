## CMake file for libwavepack based Wavpack audio reader
## vim:fdm=marker

## - Variables: PKG_REQUIRE_ARRAY, PROJECT_NAME, PROJECT_SOURCE_DIR


message (STATUS "Build with WavPack reading capability for wavpack audio" )

find_package (libwavpack 5.0.0 REQUIRED )
list (APPEND PKG_REQUIRE_ARRAY "wavpack >= 5.0.0" )

target_sources (${PROJECT_NAME} PRIVATE readerwvpk.cpp )

target_include_directories (${PROJECT_NAME} PRIVATE ${libwavpack_INCLUDE_DIRS} )
target_link_libraries      (${PROJECT_NAME} PUBLIC  ${libwavpack_LIBRARIES} )

