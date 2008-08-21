# some simple macros to inject files into bundles

macro(artoolkit_executable EXE_NAME SRCS)
	file(GLOB _datafiles "${CMAKE_SOURCE_DIR}/bin/Data/*.dat")
	file(GLOB _datafiles ${_datafiles} "${CMAKE_SOURCE_DIR}/bin/Data/*.xml")
	file(GLOB _datafiles ${_datafiles} "${CMAKE_SOURCE_DIR}/bin/Data/patt.*")
		
	if(APPLE)
		set_source_files_properties(
			${_datafiles}
			PROPERTIES
			MACOSX_PACKAGE_LOCATION "Resources/Data"
			)
			
		add_executable(${EXE_NAME} MACOSX_BUNDLE ${${SRCS}} ${_datafiles})
	else(APPLE)
		add_executable(${EXE_NAME} ${${SRCS}} ${_datafiles})
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
