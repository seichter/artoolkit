
find_package(OpenGL)
find_package(GLUT)


if(GLUT_FOUND)

	set(LIB_NAME ARgsub)

	include_directories(${CMAKE_SOURCE_DIR}/include ${GLUT_INCLUDE_DIR})

	set(HDRS ${CMAKE_SOURCE_DIR}/include/AR/gsub.h)

	file(GLOB SRCS gsub.c)

	add_library(${LIB_NAME} ${HDRS} ${SRCS})

	target_link_libraries(${LIB_NAME} AR ${OPENGL_LIBRARY} ${GLUT_LIBRARIES})
	
	artoolkit_lib_install(${LIB_NAME})
	
	include_directories(${CMAKE_SOURCE_DIR}/include ${GLUT_INCLUDE_DIR})
	
	set(LIB_NAME ARgsubUtil)

	set(HDRS ${CMAKE_SOURCE_DIR}/include/AR/gsubUtil.h)

	file(GLOB SRCS gsubUtil.c)

	add_library(${LIB_NAME} ${HDRS} ${SRCS})

	target_link_libraries(${LIB_NAME} AR)
	
	
	artoolkit_lib_install(${LIB_NAME})
	
endif(GLUT_FOUND)

SET(LIB_NAME ARgsub_lite)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

SET(HDRS ${CMAKE_SOURCE_DIR}/include/AR/gsub_lite.h)

FILE(GLOB SRCS gsub_lite.c)

ADD_LIBRARY(${LIB_NAME} ${HDRS} ${SRCS})

TARGET_LINK_LIBRARIES(${LIB_NAME} AR)

artoolkit_lib_install(${LIB_NAME})








