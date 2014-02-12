
#if(GLUT_FOUND)

#	#
#	# ARgsub
#	#
#	set(LIB_NAME ARgsub)

#	include_directories(${CMAKE_SOURCE_DIR}/include ${GLUT_INCLUDE_DIR})

#	set(HDRS ${CMAKE_SOURCE_DIR}/include/AR/gsub.h)

#	file(GLOB SRCS gsub.c)

#	add_library(${LIB_NAME} ${HDRS} ${SRCS})

#	target_link_libraries(${LIB_NAME} AR ${OPENGL_LIBRARY} ${GLUT_LIBRARIES})
	
#	artoolkit_install(${LIB_NAME})
	
#	include_directories(${CMAKE_SOURCE_DIR}/include ${GLUT_INCLUDE_DIR})
	
#	install(FILES ${HDRS}
#		DESTINATION include/AR
#	)

#	#
#	# ARgsubUtil
#	#
#	set(LIB_NAME ARgsubUtil)

#	set(HDRS ${CMAKE_SOURCE_DIR}/include/AR/gsubUtil.h)

#	file(GLOB SRCS gsubUtil.c)

#	add_library(${LIB_NAME} ${HDRS} ${SRCS})

#	target_link_libraries(${LIB_NAME} AR)
	
#	artoolkit_install(${LIB_NAME})
	
#	install(FILES ${HDRS}
#		DESTINATION include/AR
#	)
	
#endif(GLUT_FOUND)


#
# ARgsub lite
#

include_directories(${CMAKE_SOURCE_DIR}/include)

set(gsub_lighter_hdrs ${CMAKE_SOURCE_DIR}/include/AR/gsub_lighter.h)
set(gsub_lighter_srcs gsub_lighter.c)


ADD_LIBRARY(ARgsub_lighter ${gsub_lighter_hdrs} ${gsub_lighter_srcs})

TARGET_LINK_LIBRARIES(ARgsub_lighter AR)

artoolkit_install(ARgsub_lighter)

install(FILES ${gsub_lighter_hdrs}
	DESTINATION include/AR
	)



#
# ARgsubUtil
#

set(gsub_ligher_hdrs ${CMAKE_SOURCE_DIR}/include/AR/gsubUtil.h)
set(gsub_ligher_srcs gsubUtil.c)

add_library(ARgsubUtil ${gsub_ligher_hdrs} ${gsub_ligher_srcs})

target_link_libraries(ARgsubUtil AR)

artoolkit_install(ARgsubUtil)

install(FILES ${gsub_ligher_hdrs}
	DESTINATION include/AR
)










