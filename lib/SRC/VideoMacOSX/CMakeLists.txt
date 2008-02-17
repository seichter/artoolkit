SET(LIB_NAME ARvideo)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

SET(HDRS ${CMAKE_SOURCE_DIR}/include/AR/video.h)

SET(SRCS video.c ARVideoSettingsController.m ARVideoSettingsController.h videoInternal.h)

ADD_LIBRARY(${LIB_NAME} SHARED ${HDRS} ${SRCS})

TARGET_LINK_LIBRARIES(${LIB_NAME} 
	AR 
	"-framework QuickTime"
	"-framework System"
	"-framework Cocoa"
	"-framework Carbon"
	)
