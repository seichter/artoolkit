set(lib_name ARMulti)

set(SRCS
	arMultiActivate.c
	arMultiGetTransMat.c
	arMultiReadConfigFile.c
	)
	
include_directories(${CMAKE_SOURCE_DIR}/include)

set(HDRS 
	${CMAKE_SOURCE_DIR}/include/AR/arMulti.h
	)

add_library(${lib_name} STATIC ${HDRS} ${SRCS})


artoolkit_install(${lib_name})	
		
install(FILES ${HDRS}
	DESTINATION include/AR
	)
