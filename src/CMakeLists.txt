
set(TARGET smartjson)

file (GLOB HEADER_FILES *.hpp *.ipp)
file (GLOB SOURCE_FILES *.cpp)

add_library(${TARGET} ${HEADER_FILES} ${SOURCE_FILES})

if(ANDROID)
	set(PREFIX "android/${ANDROID_ABI}")
elseif(IOS)
	set(PREFIX "ios/")
elseif(CMAKE_HOST_APPLE)
	set(PREFIX "mac")
elseif(CMAKE_HOST_WIN32)
	set(PREFIX "win32")
else()
	set(PREFIX "unknown")
endif()

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib/${PREFIX})

install(FILES ${HEADER_FILES} DESTINATION ${PROJECT_BINARY_DIR}/smartjson/include)
install(TARGETS ${TARGET} DESTINATION ${PROJECT_BINARY_DIR}/smartjson/lib/${PREFIX})
