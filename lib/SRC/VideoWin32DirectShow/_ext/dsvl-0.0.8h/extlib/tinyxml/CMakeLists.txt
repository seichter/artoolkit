#
# CMake build system for DSVL 0.0.8c 
# (c) Copyrights 2007-2008 Hartmut Seichter, HIT Lab NZ
#

set(lib_name tinyxml)

set(SRCS
	tinystr.cpp
	tinyxml.cpp
	tinyxmlerror.cpp
	tinyxmlparser.cpp
	tinystr.h
	tinyxml.h
	)
	
add_definitions(-DTIXML_USE_STL)
	
add_library(${lib_name} STATIC
	${SRCS}
	)
	
if(NOT CMAKE_PROJECT_NAME MATCHES "tinyxml")
	set_target_properties(${lib_name}
		PROPERTIES
		PROJECT_LABEL "Dependency ${lib_name}"
	)
endif(NOT CMAKE_PROJECT_NAME MATCHES "tinyxml")