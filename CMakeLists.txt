##############################################################################
# CMake build system for ARToolKit 2.7.x                                     #
#                                                                            #
# (c) Copyright 2007-2008, HITLabNZ, Hartmut Seichter                        #
##############################################################################

cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

project(ARToolKit)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules" ${CMAKE_MODULE_PATH})

set(ARTOOLKIT_VERSION_MAJOR 2)
set(ARTOOLKIT_VERSION_MINOR 72)
set(ARTOOLKIT_VERSION_TINY  3)
set(ARTOOLKIT_VERSION_REV   99)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE PATH "Executable output folder")
set(LIBRARY_OUTPUT_PATH    ${CMAKE_BINARY_DIR}/lib CACHE PATH "Library output folder")

#
# Preset for Visual Studio products
#
if(MSVC)

	set(GLUT_ROOT_PATH "${CMAKE_SOURCE_DIR}/lib/SRC/_ext/glut-3.7.6-bin" CACHE PATH "GLUT root folder")
	set(GLUT_glut_LIBRARY "${CMAKE_SOURCE_DIR}/lib/SRC/_ext/glut-3.7.6-bin/lib/glut32.lib" CACHE FILENAME "GLUT include library")
	
	install(FILES
		${CMAKE_SOURCE_DIR}/lib/SRC/_ext/glut-3.7.6-bin/lib/glut32.dll
		DESTINATION bin
	)
	
endif(MSVC)

if(WIN32)
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Data
		DESTINATION bin
		PATTERN ".svn" EXCLUDE	
	)
endif(WIN32)

install(FILES 
	ChangeLog.txt
	COPYING.txt
	README.txt
	DESTINATION share/artoolkit_2
	)
			
#
# Depending on what is available we work our way through
#
find_package(OpenGL)
find_package(GLUT)


if   (APPLE)
	set(CMAKE_OSX_ARCHITECTURES ppc;i386)
	set(CMAKE_OSX_SYSROOT /Developer/SDKs/MacOSX10.4u.sdk)
endif(APPLE)

if   (UNIX)
	set(USE_GSTREAMER 0)

	include(FindGStreamer)

	if(GSTREAMER_FOUND)
		SET(USE_GSTREAMER 1)
	endif(GSTREAMER_FOUND)
endif(UNIX)

# right order of include files is important
include_directories(${CMAKE_BINARY_DIR}/include ${CMAKE_SOURCE_DIR}/include)

include(ARToolKitHelper)

add_subdirectory(lib)
add_subdirectory(util)
add_subdirectory(examples)


#
# Optional packaging for SDK
#
option(ARTOOLKIT_BUILD_PACKAGE "Build distribution packages" OFF)
if(ARTOOLKIT_BUILD_PACKAGE)
	configure_file(
		${CMAKE_SOURCE_DIR}/CMakeModules/Packaging.cmake.in
		${CMAKE_BINARY_DIR}/.cpack/ARTKPackaging.cmake	
		@ONLY
	)		
	include(${CMAKE_BINARY_DIR}/.cpack/ARTKPackaging.cmake)			
endif (ARTOOLKIT_BUILD_PACKAGE)

