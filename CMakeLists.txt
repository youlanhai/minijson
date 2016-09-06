cmake_minimum_required(VERSION 2.8)

PROJECT(SmartJson)

add_subdirectory(src)

if((NOT IOS) AND (NOT ANDROID))
	add_subdirectory(test)
endif()
