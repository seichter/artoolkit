
find_package(SDL2)
find_package(OpenGL)

if(SDL2_FOUND AND OPENGL_FOUND)

	set(srcs
		simpleLite.c
		)

	include_directories(${SDL2_INCLUDE_DIR})

	artoolkit_example(simpleSDL2 srcs)

	target_link_libraries(simpleSDL2 ${SDL2_LIBRARY} ${OPENGL_LIBRARIES})

endif()
