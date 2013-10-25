##############################################################################
# CMake build system for ARToolKit 2.7.x                                     #
#                                                                            #
# (c) Copyright 2007-2013 Hartmut Seichter                                   #
##############################################################################

cmake_minimum_required(VERSION 2.8.8)

project(ARToolKit)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules" ${CMAKE_MODULE_PATH})

set(ARTOOLKIT_VERSION_MAJOR 2)
set(ARTOOLKIT_VERSION_MINOR 8)
set(ARTOOLKIT_VERSION_TINY  0)
set(ARTOOLKIT_VERSION_REV   0613)


set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/build CACHE PATH "installation directory" FORCE)

set(ARTOOLKIT_VERSION_SHORT "${ARTOOLKIT_VERSION_MAJOR}.${ARTOOLKIT_VERSION_MINOR}")
set(ARTOOLKIT_VERSION_FULL
	"${ARTOOLKIT_VERSION_MAJOR}.${ARTOOLKIT_VERSION_MINOR}.${ARTOOLKIT_VERSION_TINY}"
	)
	
set(ARTOOLKIT_SHARE_FOLDER share/artoolkit-${ARTOOLKIT_VERSION_SHORT})


#
# apply the RPATH to be used when installing, necessary to help the
# dynamic library finding libs on ia64 systems
#
if(UNIX AND NOT APPLE)
	set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif(UNIX AND NOT APPLE)

#
# Depending on what is available we work our way through
#
find_package(OpenGL)
find_package(GLUT)
find_package(PkgConfig)


find_package(GStreamer)


#
# Optional build with OpenVRML
#
option(ARTOOLKIT_USE_OPENVRML "Create VRML based libraries and examples" OFF)
if (ARTOOLKIT_USE_OPENVRML)
	find_package(OpenVRML)
endif(ARTOOLKIT_USE_OPENVRML)

#
# Preset some of the output directories
# 
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE PATH "Executable output folder")
set(LIBRARY_OUTPUT_PATH    ${CMAKE_BINARY_DIR}/lib CACHE PATH "Library output folder")

#
# Apple presets
#
if   (APPLE)
#	set(CMAKE_OSX_ARCHITECTURES i386)
#	set(CMAKE_OSX_SYSROOT /Developer/SDKs/MacOSX10.4u.sdk CACHE PATH "SDK to link against")
endif(APPLE)

#
# Preset for Visual Studio products
#
if(MSVC)

	set(FREEGLUT_ROOT_DIR "lib/SRC/_ext/freeglut")

	# compile internal GLUT copy
	add_subdirectory(${FREEGLUT_ROOT_DIR}/src)

	#
	# Add our internal copy of FreeGLUT
	#
	set(GLUT_ROOT_PATH "${CMAKE_SOURCE_DIR}/${FREEGLUT_ROOT_DIR}" CACHE PATH "GLUT root folder")
	set(GLUT_LIBRARIES glut)
	set(GLUT_FOUND TRUE)
	
endif(MSVC)

if(WIN32)
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Data
		DESTINATION bin
		PATTERN ".svn" EXCLUDE
	)
else(WIN32)
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Data
		DESTINATION ${ARTOOLKIT_SHARE_FOLDER}
		PATTERN ".svn" EXCLUDE
		PATTERN "WDM_camera_*" EXCLUDE	
	)	
endif(WIN32)

install(FILES 
	ChangeLog.txt
	COPYING.txt
	README.txt
	DESTINATION ${ARTOOLKIT_SHARE_FOLDER}
	)

if(UNIX)
	if(GSTREAMER_INCLUDE_DIR AND GSTREAMER_LIBRARIES)
		SET(USE_GSTREAMER 1)
	endif(GSTREAMER_INCLUDE_DIR AND GSTREAMER_LIBRARIES)
endif(UNIX)

# right order of include files is important
include_directories(${CMAKE_BINARY_DIR}/include ${CMAKE_SOURCE_DIR}/include)

include(ARToolKitHelper)

add_subdirectory(src)


option(BUILD_EXAMPLES "Build examples" FALSE)
if(BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

option(BUILD_UTILS "Build utilities" FALSE)
if(BUILD_UTILS)
	add_subdirectory(util)
endif()




#
# Install the documentation
#
install(DIRECTORY doc
	DESTINATION ${ARTOOLKIT_SHARE_FOLDER}
	PATTERN ".svn" EXCLUDE
	)

#
# Install the pattern files
#	
add_subdirectory(patterns)


#
# Uninstall
#
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
  
add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")


#
# Optional packaging for SDK
#
option(ARTOOLKIT_BUILD_PACKAGE "Build distribution packages" OFF)
if(ARTOOLKIT_BUILD_PACKAGE)
	configure_file(
		${CMAKE_SOURCE_DIR}/CMakeModules/Packaging.cmake.in
		${CMAKE_BINARY_DIR}/.cpack/ARTKPackaging.cmake	
		IMMEDIATE @ONLY
	)		
	include(${CMAKE_BINARY_DIR}/.cpack/ARTKPackaging.cmake)			
endif (ARTOOLKIT_BUILD_PACKAGE)

