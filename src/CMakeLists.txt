
set(TARGET smartjson)

file (GLOB HEADER_FILES *.hpp)
file (GLOB SOURCE_FILES *.cpp)

if (MSVC)
	list(APPEND SOURCE_FILES "SmartJsonNatvisFile.natvis")
endif()

add_library(${TARGET} ${HEADER_FILES} ${SOURCE_FILES})

install(FILES ${HEADER_FILES} DESTINATION include/smartjson)
install(TARGETS ${TARGET} DESTINATION lib)
