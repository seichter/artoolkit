
set(EXE_NAME simpleVRML)

set(SRCS
	object.h
	object.c
	simpleVRML.c
	)

if(APPLE)

	set(MACOSX_BUNDLE_BUNDLE_NAME           ${EXE_NAME})
	set(MACOSX_BUNDLE_BUNDLE_VERSION        ${ARTOOLKIT_VERSION_SHORT})
	set(MACOSX_BUNDLE_SHORT_VERSION_STRING  ${ARTOOLKIT_VERSION_FULL})
	set(MACOSX_BUNDLE_LONG_VERSION_STRING   "ARToolKit ${EXE_NAME} Version ${ARTOOLKIT_VERSION_FULL}")
	set(MACOSX_BUNDLE_ICON_FILE             ARToolKit.icns)	
	set(MACOSX_BUNDLE_COPYRIGHT             "(c) 2008 Human Interface Technology Laboratory New Zealand")
	
	set_source_files_properties(
		${CMAKE_SOURCE_DIR}/share/ARToolKit.icns
		PROPERTIES
		MACOSX_PACKAGE_LOCATION "Resources"
		)
	set_source_files_properties(
		${ARTOOLKIT_FILES_DATA}
		PROPERTIES
		HEADER_FILE_ONLY TRUE
		MACOSX_PACKAGE_LOCATION "Resources/Data"
		)
	set_source_files_properties(
		${ARTOOLKIT_FILES_DATA_MULTI}
		PROPERTIES
		HEADER_FILE_ONLY TRUE
		MACOSX_PACKAGE_LOCATION "Resources/Data/multi"
		)
		
	set_source_files_properties(
		${ARTOOLKIT_FILES_VRML}
		PROPERTIES
		HEADER_FILE_ONLY TRUE
		MACOSX_PACKAGE_LOCATION "Resources/Wrl"
		)
		
	set_source_files_properties(
		${ARTOOLKIT_FILES_VRML_TEXTURES}
		PROPERTIES
		HEADER_FILE_ONLY TRUE
		MACOSX_PACKAGE_LOCATION "Resources/Wrl/textures"
		)
	
		
	set_source_files_properties(
		${OPENVRML_LIBRARIES}
		PROPERTIES
		HEADER_FILE_ONLY TRUE
		MACOSX_PACKAGE_LOCATION "MacOS"
		)
		
	add_executable(${EXE_NAME} MACOSX_BUNDLE 
		${SRCS} ${CMAKE_SOURCE_DIR}/share/ARToolKit.icns
		${ARTOOLKIT_FILES_VRML_TEXTURES}
		${ARTOOLKIT_FILES_VRML}
		${ARTOOLKIT_FILES_DATA}
		)
		
else(APPLE)
	add_executable(${EXE_NAME} ${SRCS})
endif(APPLE)


target_link_libraries(${EXE_NAME} 
	${GLUT_LIBRARIES} ${OPENGL_LIBRARIES} ${OPENVRML_LIBRARIES}
	AR ARgsub ARgsub_lite ARgsubUtil ARvrml ARvideo
	stdc++
	)


