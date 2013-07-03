##############################################################################
# CMake build system for ARToolKit 2.7.x                                     #
#                                                                            #
# (c) Copyright 2007-2013, HITLabNZ, Hartmut Seichter                        #
##############################################################################


#
# On gcc we need position independent code for the libraries
# this is required for x86_64 and recommended for x86 
#
if(UNIX AND NOT APPLE)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC"  )
endif(UNIX AND NOT APPLE)


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
	DESTINATION include/AR/sys
	)
	
install(FILES
	${CMAKE_SOURCE_DIR}/include/AR/video.h
	DESTINATION include/AR
	)


# Video
if(USE_GSTREAMER)
	add_subdirectory(VideoGStreamer)
endif(USE_GSTREAMER)


