##############################################################################
# CMake build system for FreeGLUT 2.6.x                                      #
#                                                                            #
# (c) Copyright 2007-2008  Hartmut Seichter, HITLabNZ                        #
#                                                                            #
##############################################################################


set(lib_name glut)

set(srcs
	freeglut_callbacks.c
	freeglut_cursor.c
	freeglut_display.c
	freeglut_ext.c
	freeglut_font.c
	freeglut_font_data.c
	freeglut_gamemode.c
	freeglut_geometry.c
	freeglut_glutfont_definitions.c
	freeglut_init.c
	freeglut_input_devices.c
	freeglut_joystick.c
	freeglut_main.c
	freeglut_menu.c
	freeglut_misc.c
	freeglut_overlay.c
	freeglut_state.c
	freeglut_stroke_mono_roman.c
	freeglut_stroke_roman.c
	freeglut_structure.c
	freeglut_teapot.c
	freeglut_videoresize.c
	freeglut_window.c
	)
	
set(hdrs
	freeglut_internal.h
	freeglut_teapot_data.h
	)
	
set(public_hdrs
	${CMAKE_CURRENT_SOURCE_DIR}/../include/GL/freeglut.h
	${CMAKE_CURRENT_SOURCE_DIR}/../include/GL/freeglut_ext.h
	${CMAKE_CURRENT_SOURCE_DIR}/../include/GL/freeglut_std.h
	${CMAKE_CURRENT_SOURCE_DIR}/../include/GL/glut.h
	)
	
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../include
	)
	
add_definitions(-DFREEGLUT_EXPORTS)

add_library(${lib_name} SHARED 	${srcs} ${hdrs} ${public_hdrs})


if(NOT CMAKE_PROJECT_NAME MATCHES "FreeGLUT")
	set_target_properties(${lib_name}
		PROPERTIES
		PROJECT_LABEL "Dependency ${lib_name}"
	)
endif(NOT CMAKE_PROJECT_NAME MATCHES "FreeGLUT")

target_link_libraries(${lib_name}
	${OPENGL_LIBRARIES}
	)
	
if(WIN32)
	target_link_libraries(${lib_name}
		winmm.lib
		)
	set(include_lib_dir bin)
else(WIN32)
	set(include_lib_dir lib)
endif(WIN32)
	
install(TARGETS glut
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION ${include_lib_dir}
	ARCHIVE DESTINATION lib
	)
	
install(FILES ${public_hdrs} DESTINATION include/GL)
	
	
	