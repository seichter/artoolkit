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