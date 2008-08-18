SET(EXE_NAME mk_patt)

SET(SRCS mk_patt.c)

if(GLUT_FOUND)

	add_definitions(${GLUT_DEFS})

	include_directories(${GLUT_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})

	ARTOOLKIT_EXECUTABLE(${EXE_NAME} ${SRCS})

	target_link_libraries(${EXE_NAME} AR ARgsub_lite ARvideo ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
	
	artoolkit_exe_install(${EXE_NAME})
	
endif(GLUT_FOUND)
