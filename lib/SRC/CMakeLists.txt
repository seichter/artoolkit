# Core library
add_subdirectory(AR)
add_subdirectory(ARMulti)

# GL utility
add_subdirectory(Gl)

# Video
IF   (APPLE)
	ADD_SUBDIRECTORY(VideoMacOSX)
ENDIF(APPLE)

if(WIN32)
	add_subdirectory(VideoWin32DirectShow)
endif(WIN32)

IF   (UNIX)
	IF   (USE_GSTREAMER)
		ADD_SUBDIRECTORY(VideoGStreamer)
	ENDIF(USE_GSTREAMER)
ENDIF(UNIX)
