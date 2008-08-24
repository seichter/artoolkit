##############################################################################
# CMake build system for ARToolKit 2.7.x                                     #
#                                                                            #
# (c) Copyright 2007-2008, HITLabNZ, Hartmut Seichter                        #
##############################################################################


# Core library
add_subdirectory(AR)
add_subdirectory(ARMulti)

# GL utility
add_subdirectory(Gl)

#
# ARvideo headers - just install them 
#
install(FILES	
	${CMAKE_SOURCE_DIR}/include/AR/sys/videoGStreamer.h
	${CMAKE_SOURCE_DIR}/include/AR/sys/videoLinux1394Cam.h
	${CMAKE_SOURCE_DIR}/include/AR/sys/videoLinuxDV.h
	${CMAKE_SOURCE_DIR}/include/AR/sys/videoLinuxV4L.h
	${CMAKE_SOURCE_DIR}/include/AR/sys/videoMacOSX.h
	${CMAKE_SOURCE_DIR}/include/AR/sys/videoSGI.h
	${CMAKE_SOURCE_DIR}/include/AR/sys/videoWin32DirectShow.h
	DESTINATION include/AR/sys
	)
	
install(FILES
	${CMAKE_SOURCE_DIR}/include/AR/video.h
	DESTINATION include/AR
	)


# Video
if(APPLE)
	add_subdirectory(VideoMacOSX)
endif(APPLE)

if(WIN32)
	add_subdirectory(VideoWin32DirectShow)
endif(WIN32)

if(UNIX)
	if(USE_GSTREAMER)
		add_subdirectory(VideoGStreamer)
	endif(USE_GSTREAMER)
endif(UNIX)


if(OPENVRML_FOUND)
	find_package(OpenVRML)	
	if(ARTOOLKIT_USE_OPENVRML)
		add_subdirectory(ARvrml)
	endif(ARTOOLKIT_USE_OPENVRML)
endif(OPENVRML_FOUND)


