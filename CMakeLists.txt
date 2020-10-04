cmake_minimum_required(VERSION 3.10)
project(oslib C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(SOURCE_FILES
		src/platform.c
		src/win32/hot_reload.c
		src/win32/io.c
		src/win32/timer.c)
endif()

set (OSLIB_IO_EXAMPLE_SOURCE_FILES sandbox/io/oslib_io.c)
set (OSLIB_HOT_RELOAD_EXAMPLE_SOURCE_FILES sandbox/hot_reload/oslib_hot_reload.c)
set (OSLIB_HOT_RELOAD_EXMAPLE_DLL_SOURCE_FILES sandbox/hot_reload/testlib.c)


add_library(oslib STATIC ${SOURCE_FILES})
set_target_properties(oslib PROPERTIES PREFIX "")

add_executable(oslib_io ${OSLIB_IO_EXAMPLE_SOURCE_FILES})
target_link_libraries(oslib_io oslib)
add_executable(oslib_hotreload ${OSLIB_HOT_RELOAD_EXAMPLE_SOURCE_FILES})
add_library(testlib SHARED ${OSLIB_HOT_RELOAD_EXMAPLE_DLL_SOURCE_FILES})
target_link_libraries(oslib_hotreload oslib)