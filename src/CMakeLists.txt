
set(TARGET smartjson)

file (GLOB HEADER_FILES *.hpp *.ipp)
file (GLOB SOURCE_FILES *.cpp)

add_library(${TARGET} ${HEADER_FILES} ${SOURCE_FILES})
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
