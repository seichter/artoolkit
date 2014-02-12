# some simple macros to inject files into bundles

set(ARTOOLKIT_FILES_DATA
	${CMAKE_SOURCE_DIR}/bin/Data/camera_para.dat
	${CMAKE_SOURCE_DIR}/bin/Data/object_data
#	${CMAKE_SOURCE_DIR}/bin/Data/object_data_vrml
	${CMAKE_SOURCE_DIR}/bin/Data/object_data2
	${CMAKE_SOURCE_DIR}/bin/Data/paddle_data
	${CMAKE_SOURCE_DIR}/bin/Data/patt.calib
	${CMAKE_SOURCE_DIR}/bin/Data/patt.hiro
	${CMAKE_SOURCE_DIR}/bin/Data/patt.kanji
	${CMAKE_SOURCE_DIR}/bin/Data/patt.sample1
	${CMAKE_SOURCE_DIR}/bin/Data/patt.sample2
	${CMAKE_SOURCE_DIR}/bin/Data/patt.calib
#	${CMAKE_SOURCE_DIR}/bin/Data/WDM_camera_flipV.xml
#	${CMAKE_SOURCE_DIR}/bin/Data/WDM_camera.xml
)

set(ARTOOLKIT_FILES_VRML
#	${CMAKE_SOURCE_DIR}/bin/Wrl/bud_B.dat
#	${CMAKE_SOURCE_DIR}/bin/Wrl/snoman.dat
#	${CMAKE_SOURCE_DIR}/bin/Wrl/bud_B.wrl
#	${CMAKE_SOURCE_DIR}/bin/Wrl/snoman.wrl
)

set(ARTOOLKIT_FILES_VRML_TEXTURES
#	${CMAKE_SOURCE_DIR}/bin/Wrl/textures/bud_B_Fractal_2.gif
#	${CMAKE_SOURCE_DIR}/bin/Wrl/textures/bud_B_Ramp_6.gif
#	${CMAKE_SOURCE_DIR}/bin/Wrl/textures/bud_B_Ramp_2.gif
#	${CMAKE_SOURCE_DIR}/bin/Wrl/textures/bud_B_Ramp_4.gif
#	${CMAKE_SOURCE_DIR}/bin/Wrl/textures/bud_B_Ramp_5.gif
)

set(ARTOOLKIT_FILES_DATA_WIN32
#	${CMAKE_SOURCE_DIR}/bin/Data/WDM_camera_flipV.xml
#	${CMAKE_SOURCE_DIR}/bin/Data/WDM_camera.xml
)

set(ARTOOLKIT_FILES_DATA_MULTI
	${CMAKE_SOURCE_DIR}/bin/Data/multi/marker.dat
	${CMAKE_SOURCE_DIR}/bin/Data/multi/patt.a
	${CMAKE_SOURCE_DIR}/bin/Data/multi/patt.b
	${CMAKE_SOURCE_DIR}/bin/Data/multi/patt.c
	${CMAKE_SOURCE_DIR}/bin/Data/multi/patt.d
	${CMAKE_SOURCE_DIR}/bin/Data/multi/patt.f
	${CMAKE_SOURCE_DIR}/bin/Data/multi/patt.g
)


macro(artoolkit_executable EXE_NAME SRCS)
	
	if(APPLE)
	
		set(MACOSX_BUNDLE_BUNDLE_NAME           ${EXE_NAME})
		set(MACOSX_BUNDLE_BUNDLE_VERSION        ${ARTOOLKIT_VERSION_SHORT})
		set(MACOSX_BUNDLE_SHORT_VERSION_STRING  ${ARTOOLKIT_VERSION_FULL})
		set(MACOSX_BUNDLE_LONG_VERSION_STRING   "ARToolKit ${EXE_NAME} Version ${ARTOOLKIT_VERSION_FULL}")
		set(MACOSX_BUNDLE_ICON_FILE             ARToolKit.icns)	
		set(MACOSX_BUNDLE_COPYRIGHT             "(c) 2013 Human Interface Technology Laboratory New Zealand")
		
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
			
		add_executable(${EXE_NAME} MACOSX_BUNDLE 
			${${SRCS}} ${CMAKE_SOURCE_DIR}/share/ARToolKit.icns
			${ARTOOLKIT_FILES_DATA}
			${ARTOOLKIT_FILES_DATA_MULTI}
			)
			
	else(APPLE)
		set(TEMP_RC_NAME ${CMAKE_BINARY_DIR}/.rc/${EXE_NAME}_temp.rc)
		
		configure_file(
			${CMAKE_SOURCE_DIR}/share/ARToolKit.rc.in
			${TEMP_RC_NAME}
			IMMEDIATE @ONLY
		)
	
		add_executable(${EXE_NAME} ${${SRCS}} ${TEMP_RC_NAME})
	endif(APPLE)
endmacro(artoolkit_executable)


macro(artoolkit_install target)

	if   (WIN32)
		set(lib_dest bin)
	else (WIN32)
		set(lib_dest lib)
	endif(WIN32)
	
	set(cmake_version_short "${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}")

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
		
endmacro(artoolkit_install target)



macro(artoolkit_lib_install target)

	artoolkit_install(target)
		
endmacro(artoolkit_lib_install target)


macro(artoolkit_example name source_files)

	set(exe_name ${name})

	include_directories(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})

	artoolkit_executable(${exe_name} ${source_files})

	target_link_libraries(${exe_name}
		AR
		ARvideo
		ARMulti
		ARGLUtils
		)

	set_target_properties(${exe_name}
		PROPERTIES
		PROJECT_LABEL "Example ${name}"
		)

	artoolkit_install(${exe_name})

endmacro(artoolkit_example name source_files)


macro(artoolkit_example_lite name source_files)

	set(exe_name ${name})
	
	include_directories(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})

	artoolkit_executable(${exe_name} ${source_files})

	target_link_libraries(${exe_name} 
		AR 
		ARvideo
		ARMulti
		ARgsubUtil
		ARgsub_lite
		${OPENGL_LIBRARIES} ${GLUT_LIBRARIES}
		)
		
	set_target_properties(${exe_name}
		PROPERTIES
		PROJECT_LABEL "Example ${name}"
	)

	artoolkit_install(${exe_name})	

endmacro(artoolkit_example_lite name source_files)


macro(artoolkit_example_glut name source_files)

	set(exe_name ${name})
	
	include_directories(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})
	
	artoolkit_executable(${exe_name} ${source_files})

	target_link_libraries(${exe_name} 
		AR 		
		ARvideo
		ARgsub
		ARgsubUtil		
		ARMulti
		${OPENGL_LIBRARIES} ${GLUT_LIBRARIES}
		)
		
	set_target_properties(${exe_name}
		PROPERTIES
		PROJECT_LABEL "Example ${name}"
	)

	artoolkit_install(${exe_name})	

endmacro(artoolkit_example_glut name source_files)


macro(artoolkit_utility_glut name source_files)

	set(exe_name ${name})
	
	include_directories(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})
	
	artoolkit_executable(${exe_name} ${source_files})

	target_link_libraries(${exe_name} 
		AR 
		ARvideo
		ARMulti		
		ARgsub
		ARgsub_lite
		ARgsubUtil
		${OPENGL_LIBRARIES} ${GLUT_LIBRARIES}
		)
		
	set_target_properties(${exe_name}
		PROPERTIES
		PROJECT_LABEL "Utility ${name}"
	)

	artoolkit_install(${exe_name})	

endmacro(artoolkit_utility_glut name source_files)


macro(artoolkit_framework)
	
	

endmacro(artoolkit_framework)

