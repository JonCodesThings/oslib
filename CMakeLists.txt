cmake_minimum_required(VERSION 3.10)
project(oslib C)

set(CMAKE_C_STANDARD 11)
option(BUILD_SAMPLES "Build sample programs" OFF)

include_directories(.)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(SOURCE_FILES
		src/platform.c
		src/win32/hot_reload.c
		src/win32/io.c
		src/win32/timer.c
		src/win32/network.c)
endif()

if (BUILD_SAMPLES)
set (OSLIB_IO_EXAMPLE_SOURCE_FILES sandbox/io/oslib_io.c)
set (OSLIB_HOT_RELOAD_EXAMPLE_SOURCE_FILES sandbox/hot_reload/oslib_hot_reload.c)
set (OSLIB_HOT_RELOAD_EXMAPLE_DLL_SOURCE_FILES sandbox/hot_reload/testlib.c)
set (OSLIB_NETWORK_EXAMPLE_CLIENT sandbox/network/oslib_network_client.c)
set (OSLIB_NETWORK_EXAMPLE_SERVER sandbox/network/oslib_network_server.c)
set (OSLIB_NETWORK_EXAMPLE_HTTP sandbox/network/oslib_network_http.c)
endif()


add_library(oslib STATIC ${SOURCE_FILES})
set_target_properties(oslib PROPERTIES PREFIX "")

if (BUILD_SAMPLES)
add_executable(oslib_io ${OSLIB_IO_EXAMPLE_SOURCE_FILES})
target_link_libraries(oslib_io oslib)
add_executable(oslib_hotreload ${OSLIB_HOT_RELOAD_EXAMPLE_SOURCE_FILES})
add_library(testlib SHARED ${OSLIB_HOT_RELOAD_EXMAPLE_DLL_SOURCE_FILES})
target_link_libraries(oslib_hotreload oslib)
add_executable(oslib_network_client ${OSLIB_NETWORK_EXAMPLE_CLIENT})
add_executable(oslib_network_server ${OSLIB_NETWORK_EXAMPLE_SERVER})
add_executable(oslib_network_http ${OSLIB_NETWORK_EXAMPLE_HTTP})
target_link_libraries(oslib_network_client oslib)
target_link_libraries(oslib_network_server oslib)
target_link_libraries(oslib_network_http oslib)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
target_link_libraries(oslib_network_client "ws2_32.lib")
target_link_libraries(oslib_network_server "ws2_32.lib")
target_link_libraries(oslib_network_http "ws2_32.lib")
endif()
endif()