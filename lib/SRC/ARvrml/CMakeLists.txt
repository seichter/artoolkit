#
# CMake build system for ARToolKit 
#
# (c) Copyrights 2007-2008 Hartmut Seichter, HIT Lab NZ
#

set(lib_name ARvrml)

set(OPENVRML_API)

#
# We have to go through some convoluted hops to 
# determine the correct version.
# Assumption is: nothing new than 0.16.x 
#
if(OPENVRML_FOUND AND PKG_CONFIG_FOUND)
	pkg_search_module(LIBOPENVRML openvrml)

	if   (${LIBOPENVRML_VERSION} MATCHES "0.14.")
		set(OPENVRML_API "_openvrml-0.14.3")
	endif(${LIBOPENVRML_VERSION} MATCHES "0.14.")
	
	if   (${LIBOPENVRML_VERSION} MATCHES "0.15.")
		set(OPENVRML_API "_openvrml-0.15.10")
	endif(${LIBOPENVRML_VERSION} MATCHES "0.15.")

	if   (${LIBOPENVRML_VERSION} MATCHES "0.16.")
		set(OPENVRML_API "")
	endif(${LIBOPENVRML_VERSION} MATCHES "0.16.")

endif(OPENVRML_FOUND AND PKG_CONFIG_FOUND)

#
# If we have no pkg-config we only assume 0.16 is supported
#

set(srcs_openvrml
	arViewerCapi${OPENVRML_API}.cpp
	arViewer${OPENVRML_API}.cpp
	arViewer${OPENVRML_API}.h
	)

set(srcs_common
	arViewer.h
	)

set(hdrs_public
	${CMAKE_SOURCE_DIR}/include/AR/arvrml.h
	)

include_directories(${CMAKE_SOURCE_DIR}/include ${GLUT_INCLUDE_DIR} ${OPENVRML_INCLUDE_DIR})

if(WIN32)
	add_definitions(-DOPENVRML_USE_DLL -DOPENVRML_GL_USE_DLL)
endif(WIN32)


add_library(${lib_name} ${hdrs_public} ${srcs_openvrml})

target_link_libraries(${lib_name} 
	AR 
	${OPENVRML_LIBRARIES} 
	${OPENGL_LIBRARY} 
	${GLUT_LIBRARIES}
	)
	
artoolkit_install(${lib_name})


install(FILES ${hdrs_public}
	DESTINATION include/AR
)



