# some simple macros to inject files into bundles

MACRO(ARTOOLKIT_EXECUTABLE EXE_NAME SRCS)
	FILE(GLOB _datafiles "${CMAKE_SOURCE_DIR}/bin/Data/*.dat")
	FILE(GLOB _datafiles ${_datafiles} "${CMAKE_SOURCE_DIR}/bin/Data/*.xml")
	FILE(GLOB _datafiles ${_datafiles} "${CMAKE_SOURCE_DIR}/bin/Data/patt.*")
	IF(APPLE)
		SET_SOURCE_FILES_PROPERTIES(
			${_datafiles}
			PROPERTIES
			MACOSX_PACKAGE_LOCATION "Resources/Data"
			)
			
		ADD_EXECUTABLE(${EXE_NAME} MACOSX_BUNDLE ${SRCS} ${_datafiles})
	ELSE(APPLE)
		ADD_EXECUTABLE(${EXE_NAME} ${SRCS} ${_datafiles})
	ENDIF(APPLE)
ENDMACRO(ARTOOLKIT_EXECUTABLE)

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

