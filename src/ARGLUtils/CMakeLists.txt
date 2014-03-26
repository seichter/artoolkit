include_directories(${CMAKE_SOURCE_DIR}/include)

set(gsub_lighter_hdrs ${CMAKE_SOURCE_DIR}/include/AR/arGLUtils.h)
set(gsub_lighter_srcs glutils.c)


add_library(ARGLUtils ${gsub_lighter_hdrs} ${gsub_lighter_srcs})

#set_target_properties(ARGLUtils
#	PROPERTIES
#	POSITION_INDEPENDENT_CODE TRUE
#	)

target_link_libraries(ARGLUtils AR)

artoolkit_install(ARGLUtils)

install(FILES ${gsub_lighter_hdrs}
	DESTINATION include/AR
	)
