## Root CMake file for readerwav
## vim:fdm=marker
##
## Prerequisites from PARENT
##
## - Variables: PROJECT_NAME

target_sources (${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/readerwav.cpp )

