set(EXE_NAME simpleLite)

set(SRCS simpleLite.c)

include_directories(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})

artoolkit_executable(${EXE_NAME} ${SRCS})

target_link_libraries(${EXE_NAME} AR ARgsub_lite ARvideo ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

artoolkit_exe_install(${EXE_NAME})
