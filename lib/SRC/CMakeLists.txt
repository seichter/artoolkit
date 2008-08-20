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
