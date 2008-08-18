#
# CMake build system for DSVL 0.0.8c 
# (c) Copyrights 2007-2008 Hartmut Seichter, HIT Lab NZ
#

set(lib_name DSVL)

find_package(DirectShow)

set(SRCS
	DSVL.cpp
	DSVL_GraphManager.cpp
	DSVL_Helpers.cpp
	DSVL_PixelFormat.cpp
	IEEE1394_id.cpp
	DSVL.h
	DSVL_GraphManager.h
	DSVL_Helpers.h
	DSVL_PixelFormat.h
	DSVL_PixelFormatTypes.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${DIRECTSHOW_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../extlib/tinyxml
	)

add_definitions(-DDSVL_EXPORTS -DTIXML_USE_STL)

add_library(DSVL SHARED ${SRCS})

target_link_libraries(DSVL tinyxml ${DIRECTSHOW_LIBRARIES})

if   (${CMAKE_BUILD_TYPE} MATCHES Debug)
	target_link_libraries(DSVL comsuppd comsuppwd)
else (${CMAKE_BUILD_TYPE} MATCHES Debug)
	target_link_libraries(DSVL comsupp comsuppw)
endif(${CMAKE_BUILD_TYPE} MATCHES Debug)

install(TARGETS DSVL
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
)


