
find_package(OpenGL)
find_package(GLUT)


if(GLUT_FOUND)

	#
	# ARgsub
	#
	set(LIB_NAME ARgsub)

	include_directories(${CMAKE_SOURCE_DIR}/include ${GLUT_INCLUDE_DIR})

	set(HDRS ${CMAKE_SOURCE_DIR}/include/AR/gsub.h)

	file(GLOB SRCS gsub.c)

	add_library(${LIB_NAME} ${HDRS} ${SRCS})

	target_link_libraries(${LIB_NAME} AR ${OPENGL_LIBRARY} ${GLUT_LIBRARIES})
	
	artoolkit_install(${LIB_NAME})
	
	include_directories(${CMAKE_SOURCE_DIR}/include ${GLUT_INCLUDE_DIR})
	
	install(FILES ${HDRS}
		DESTINATION include/AR
	)

	#
	# ARgsubUtil
	#
	set(LIB_NAME ARgsubUtil)

	set(HDRS ${CMAKE_SOURCE_DIR}/include/AR/gsubUtil.h)

	file(GLOB SRCS gsubUtil.c)

	add_library(${LIB_NAME} ${HDRS} ${SRCS})

	target_link_libraries(${LIB_NAME} AR)	
	
	artoolkit_install(${LIB_NAME})
	
	install(FILES ${HDRS}
		DESTINATION include/AR
	)
	
endif(GLUT_FOUND)


#
# ARgsub lite
#

SET(LIB_NAME ARgsub_lite)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

SET(HDRS ${CMAKE_SOURCE_DIR}/include/AR/gsub_lite.h)

FILE(GLOB SRCS gsub_lite.c)

ADD_LIBRARY(${LIB_NAME} ${HDRS} ${SRCS})

TARGET_LINK_LIBRARIES(${LIB_NAME} AR)

artoolkit_install(${LIB_NAME})

install(FILES ${HDRS}
	DESTINATION include/AR
	)










