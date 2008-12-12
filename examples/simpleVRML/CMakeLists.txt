
set(EXE_NAME simpleVRML)


include_directories(${GLUT_INCLUDE_DIR})

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
		
	list(APPEND 
		${SRCS}
		${CMAKE_SOURCE_DIR}/share/ARToolKit.icns
		${ARTOOLKIT_FILES_VRML_TEXTURES}
		${ARTOOLKIT_FILES_VRML}
		${ARTOOLKIT_FILES_DATA}		
		)
		
else(APPLE)
	## add_executable(${EXE_NAME} ${SRCS})
endif(APPLE)

artoolkit_example_glut(${EXE_NAME} SRCS)


target_link_libraries(${EXE_NAME} 
	${GLUT_LIBRARIES} ${OPENGL_LIBRARIES} ${OPENVRML_LIBRARIES}
	AR ARgsub ARgsub_lite ARgsubUtil ARvrml ARvideo
	)
	
if (CMAKE_COMPILER_IS_GNUCC)
	target_link_libraries(${EXE_NAME} 
		stdc++
	)
endif (CMAKE_COMPILER_IS_GNUCC)


set(target ${EXE_NAME})

if (cmake_version_short GREATER "2.5")
	install(TARGETS ${target}
		BUNDLE DESTINATION "/Applications/ARToolKit-${ARTOOLKIT_VERSION_FULL}"
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION ${lib_dest}	
		ARCHIVE DESTINATION lib
		PUBLIC_HEADER DESTINATION include/AR
		PERMISSIONS 
		OWNER_EXECUTE OWNER_WRITE OWNER_READ
		GROUP_EXECUTE GROUP_READ
		WORLD_EXECUTE WORLD_READ
		)
else(cmake_version_short GREATER "2.5")
	install(TARGETS ${target}
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION ${lib_dest}	
		ARCHIVE DESTINATION lib
		PERMISSIONS 
		OWNER_EXECUTE OWNER_WRITE OWNER_READ
		GROUP_EXECUTE GROUP_READ
		WORLD_EXECUTE WORLD_READ
		)
endif(cmake_version_short GREATER "2.5")

if(WIN32)

	get_filename_component(OPENVRML_LIB_PATH ${OPENVRML_openvrml_LIBRARY} PATH)
	file(GLOB OPENVRML_DLLS ${OPENVRML_LIB_PATH}/../bin/*${CMAKE_SHARED_LIBRARY_SUFFIX})


	install(FILES ${ARTOOLKIT_FILES_VRML_TEXTURES}	
		DESTINATION bin/Wrl/textures
	)
	install(FILES ${ARTOOLKIT_FILES_VRML}	
		DESTINATION bin/Wrl
	)
	
	install(
		FILES ${OPENVRML_LIBRARIES} 
		DESTINATION lib
		)
		
	install(FILES ${OPENVRML_DLLS}
		DESTINATION bin		
		)
		
endif(WIN32)

	


