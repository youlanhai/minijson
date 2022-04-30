cmake_minimum_required(VERSION 3.1)

PROJECT(smartjson)

option(SJ_BUILD_TESTS "Build the test programs" ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_STANDARD 11)

add_subdirectory(src)

if (SJ_BUILD_TESTS)
	add_subdirectory(test)
endif()
