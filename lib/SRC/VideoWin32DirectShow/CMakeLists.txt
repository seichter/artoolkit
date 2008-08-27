#
# CMake build system for ARToolKit / DSVL capture
# (c) Copyrights 2007-2008 Hartmut Seichter, HIT Lab NZ
#

add_subdirectory(_ext)

set(LIB_NAME ARvideo)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/_ext/dsvl-0.0.8h/src	
	)

set(HDRS ${CMAKE_SOURCE_DIR}/include/AR/video.h)

set(SRCS videoWin32DirectShow.cpp)

add_definitions(-DLIBARVIDEO_EXPORTS)

ADD_LIBRARY(${LIB_NAME} SHARED ${HDRS} ${SRCS})

target_link_libraries(${LIB_NAME} 
	AR 
	DSVL
	)
	
artoolkit_install(${LIB_NAME})