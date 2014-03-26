
find_package(glfw)
find_package(OpenGL)

if(GLFW_LIBRARY AND OPENGL_FOUND)

	set(srcs
		simpleGLFW.c
		)

	include_directories(${GLFW_INCLUDE_DIR})

	artoolkit_example(simpleGLFW srcs)

	target_link_libraries(simpleGLFW ${GLFW_LIBRARY} ${OPENGL_LIBRARIES})

endif()
