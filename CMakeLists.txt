##############################################################################
# CMake build system for ARToolKit 2.7.x                                     #
#                                                                            #
# (c) Copyright 2007-2008, HITLabNZ, Hartmut Seichter                        #
##############################################################################

PROJECT(ARToolKit)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules" ${CMAKE_MODULE_PATH})

SET(ARTOOLKIT_VERSION_MAJOR 2)
SET(ARTOOLKIT_VERSION_MINOR 72)
SET(ARTOOLKIT_VERSION_TINY  3)
SET(ARTOOLKIT_VERSION_REV   99)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH    ${CMAKE_BINARY_DIR}/lib)


IF   (APPLE)
	SET(CMAKE_OSX_ARCHITECTURES ppc;i386)
	SET(CMAKE_OSX_SYSROOT /Developer/SDKs/MacOSX10.4u.sdk)
ENDIF(APPLE)

IF   (UNIX)
	SET(USE_GSTREAMER 0)

	INCLUDE(FindGStreamer)

	IF(GSTREAMER_FOUND)
		SET(USE_GSTREAMER 1)
	ENDIF(GSTREAMER_FOUND)
ENDIF(UNIX)

# right order of include files is important
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/include ${CMAKE_SOURCE_DIR}/include)

INCLUDE(ARToolKitHelper)

ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(util)
ADD_SUBDIRECTORY(examples)
