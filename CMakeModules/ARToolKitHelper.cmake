# some simple macros to inject files into bundles

macro(artoolkit_executable EXE_NAME SRCS)
	set(_datafiles 
		${CMAKE_SOURCE_DIR}/bin/Data/camera_para.dat
		${CMAKE_SOURCE_DIR}/bin/Data/object_data
		${CMAKE_SOURCE_DIR}/bin/Data/object_data2
		${CMAKE_SOURCE_DIR}/bin/Data/paddle_data
		${CMAKE_SOURCE_DIR}/bin/Data/patt.calib
		${CMAKE_SOURCE_DIR}/bin/Data/patt.hiro
		${CMAKE_SOURCE_DIR}/bin/Data/patt.kanji
		${CMAKE_SOURCE_DIR}/bin/Data/patt.sample1
		${CMAKE_SOURCE_DIR}/bin/Data/patt.sample2
		${CMAKE_SOURCE_DIR}/bin/Data/patt.calib
		${CMAKE_SOURCE_DIR}/bin/Data/WDM_camera_flipV.xml
		${CMAKE_SOURCE_DIR}/bin/Data/WDM_camera.xml
	)

	set(_datafiles_multi 
		${CMAKE_SOURCE_DIR}/bin/Data/multi/marker.dat
		${CMAKE_SOURCE_DIR}/bin/Data/multi/patt.a
		${CMAKE_SOURCE_DIR}/bin/Data/multi/patt.b
		${CMAKE_SOURCE_DIR}/bin/Data/multi/patt.c
		${CMAKE_SOURCE_DIR}/bin/Data/multi/patt.d
		${CMAKE_SOURCE_DIR}/bin/Data/multi/patt.f
		${CMAKE_SOURCE_DIR}/bin/Data/multi/patt.g
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
			${_datafiles}
			PROPERTIES
			HEADER_FILE_ONLY TRUE
			MACOSX_PACKAGE_LOCATION "Resources/Data"
			)
		set_source_files_properties(
			${_datafiles_multi}
			PROPERTIES
			HEADER_FILE_ONLY TRUE
			MACOSX_PACKAGE_LOCATION "Resources/Data/multi"
			)
			
		add_executable(${EXE_NAME} MACOSX_BUNDLE 
			${${SRCS}} ${_datafiles} ${_datafiles_multi}
			${CMAKE_SOURCE_DIR}/share/ARToolKit.icns
			)
			
	else(APPLE)
		add_executable(${EXE_NAME} ${${SRCS}} ${_datafiles} ${_datafiles_multi})
	endif(APPLE)
endmacro(artoolkit_executable)

macro(artoolkit_lib_install target)

	if   (WIN32)
		set(lib_dest bin)
	else (WIN32)
		set(lib_dest lib)
	endif(WIN32)
		

	install(TARGETS ${target}
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION ${lib_dest}	
		RUNTIME DESTINATION bin
		PUBLIC_HEADER DESTINATION include/AR
		)
		
endmacro(artoolkit_lib_install target)

macro(artoolkit_exe_install target)

	if   (WIN32)
		set(lib_dest bin)
	else (WIN32)
		set(lib_dest lib)
	endif(WIN32)

	install(TARGETS ${target}
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION ${lib_dest}	
		RUNTIME DESTINATION bin
		PUBLIC_HEADER DESTINATION include/AR
		BUNDLE DESTINATION /Applications/ARToolKit-${ARTOOLKIT_VERSION_FULL}
		)
		
endmacro(artoolkit_exe_install target)


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

	artoolkit_exe_install(${exe_name})	

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

	artoolkit_exe_install(${exe_name})	

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

	artoolkit_exe_install(${exe_name})	

endmacro(artoolkit_utility_glut name source_files)
