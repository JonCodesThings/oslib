cmake_minimum_required(VERSION 3.10)
project(oslib C)

set(CMAKE_C_STANDARD 11)
option(BUILD_TESTS "Build test programs" OFF)

include_directories(.)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(SOURCE_FILES
		src/platform.c
		src/win32/hot_reload.c
		src/win32/io.c
		src/win32/timer.c)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set (SOURCE_FILES
		src/platform.c
		src/posix/hot_reload.c
		src/posix/io.c
		src/posix/timer.c)
endif()

set (OSLIB_IO_EXAMPLE_SOURCE_FILES sandbox/io/oslib_io.c)
set (OSLIB_HOT_RELOAD_EXAMPLE_SOURCE_FILES sandbox/hot_reload/oslib_hot_reload.c)
set (OSLIB_HOT_RELOAD_EXMAPLE_DLL_SOURCE_FILES sandbox/hot_reload/testlib.c)

if (BUILD_TESTS)
	enable_testing()
	file(GLOB_RECURSE TESTFILES "test/*.c")
	foreach(TESTFILE ${TESTFILES})
		get_filename_component(TESTFILE_STRIPPED ${TESTFILE} NAME_WE)
		add_executable(${TESTFILE_STRIPPED} ${TESTFILE})
		target_link_libraries(${TESTFILE_STRIPPED} oslib)
		add_test(NAME ${TESTFILE_STRIPPED} WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COMMAND ${TESTFILE_STRIPPED})
	endforeach()
endif()

add_library(oslib STATIC ${SOURCE_FILES})
set_target_properties(oslib PROPERTIES PREFIX "")
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	target_link_libraries(oslib "dl")
endif()

add_executable(oslib_io ${OSLIB_IO_EXAMPLE_SOURCE_FILES})
target_link_libraries(oslib_io oslib)
add_executable(oslib_hotreload ${OSLIB_HOT_RELOAD_EXAMPLE_SOURCE_FILES})
add_library(testlib SHARED ${OSLIB_HOT_RELOAD_EXMAPLE_DLL_SOURCE_FILES})
target_link_libraries(oslib_hotreload oslib)
