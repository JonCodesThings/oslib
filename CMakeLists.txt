cmake_minimum_required(VERSION 3.10)
project(oslib C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(SOURCE_FILES
		src/win32/hot_reload.c
		src/win32/io.c)
endif()

set (OSLIB_IO_EXAMPLE_SOURCE_FILES sandbox/io/oslib_io.c)

add_library(oslib STATIC ${SOURCE_FILES})
set_target_properties(oslib PROPERTIES PREFIX "")

add_executable(oslib_io ${OSLIB_IO_EXAMPLE_SOURCE_FILES})
target_link_libraries(oslib_io oslib)