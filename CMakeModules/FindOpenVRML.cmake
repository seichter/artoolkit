# Locate openvml library
# This module defines
# OPENVRML_LIBRARY
# OPENVRML_FOUND, if false, do not try to link to vrml 
# OPENVRML_INCLUDE_DIR, where to find the headers
#
# $OPENVRML_DIR is an environment variable that would
# correspond to the ./configure --prefix=$OPENVRML_DIR
#
# Created by Robert Osfield. 
# Modified for the debug library by Jean-S?stien Guay.


set(OPENVRML_LIB_SEARCH_PATHS
	${CMAKE_SOURCE_DIR}/../openvrml-0.16.6-bin-macosx/lib
    $ENV{OPENVRML_DIR}/lib
    $ENV{OPENVRML_DIR}
    $ENV{OSGDIR}/lib
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/lib
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
    /usr/freeware/lib64
	)

FIND_PATH(OPENVRML_INCLUDE_DIR openvrml/system.h
	${CMAKE_SOURCE_DIR}/../openvrml-0.16.6-bin-macosx/include
    $ENV{OPENVRML_DIR}/include
    $ENV{OPENVRML_DIR}
    $ENV{OSGDIR}/include
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/include
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include
    /usr/freeware/include
)

FIND_LIBRARY(OPENVRML_jpeg_LIBRARY 
    NAMES jpeg
    PATHS
	${OPENVRML_LIB_SEARCH_PATHS}
)

FIND_LIBRARY(OPENVRML_png_LIBRARY 
    NAMES png
    PATHS
	${OPENVRML_LIB_SEARCH_PATHS}
)

FIND_LIBRARY(OPENVRML_gl_LIBRARY 
    NAMES openvrml-gl
    PATHS
	${OPENVRML_LIB_SEARCH_PATHS}
)

FIND_LIBRARY(OPENVRML_js_LIBRARY 
    NAMES js
    PATHS
	${OPENVRML_LIB_SEARCH_PATHS}
)

FIND_LIBRARY(OPENVRML_boost_LIBRARY 
    NAMES boost_thread-1_33_1
    PATHS
	${OPENVRML_LIB_SEARCH_PATHS}
)

FIND_LIBRARY(OPENVRML_plds4_LIBRARY 
    NAMES plds4
    PATHS
	${OPENVRML_LIB_SEARCH_PATHS}
)

FIND_LIBRARY(OPENVRML_nspr4_LIBRARY 
    NAMES nspr4
    PATHS
	${OPENVRML_LIB_SEARCH_PATHS}
)

FIND_LIBRARY(OPENVRML_plc4_LIBRARY 
    NAMES plc4
    PATHS
	${OPENVRML_LIB_SEARCH_PATHS}
)

FIND_LIBRARY(OPENVRML_LIBRARY_DEBUG 
    NAMES openvrmld
    PATHS
    $ENV{OPENVRML_DIR}/lib
    $ENV{OPENVRML_DIR}
    $ENV{OSGDIR}/lib
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/lib
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
    /usr/freeware/lib64
)

SET(OPENVRML_FOUND "NO")
IF(OPENVRML_openvrml_LIBRARY AND OPENVRML_INCLUDE_DIR)
    SET(OPENVRML_FOUND "YES")
	SET(OPENVRML_LIBRARIES
		${OPENVRML_openvrml_LIBRARY}
		${OPENVRML_gl_LIBRARY}
		)
	if(APPLE)
		list(APPEND
			${OPENVRML_jpeg_LIBRARY}
			${OPENVRML_boost_LIBRARY}
			${OPENVRML_js_LIBRARY}
			${OPENVRML_png_LIBRARY}
			${OPENVRML_plc4_LIBRARY}
			${OPENVRML_nspr4_LIBRARY}
			${OPENVRML_plds4_LIBRARY}
		)
	endif(APPLE)
ENDIF(OPENVRML_openvrml_LIBRARY AND OPENVRML_INCLUDE_DIR)



#-------------- next part --------------
IF (WIN32)
    INCLUDE_DIRECTORIES( ${OPENVRML_INCLUDE_DIR} ${OPENVRML_INCLUDE_DIR}/openvrml ${JPEG_INCLUDE_DIR} ${PNG_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
    SET(OPENVRML_ANTLR_LIBRARY       antlr.lib)
    SET(OPENVRML_ANTLR_LIBRARY_DEBUG antlrd.lib)
    SET(OPENVRML_REGEX_LIBRARY       regex.lib)
    SET(OPENVRML_REGEX_LIBRARY_DEBUG regexd.lib)
        SET(TARGET_LIBRARIES_VARS OPENVRML_ANTLR_LIBRARY OPENVRML_REGEX_LIBRARY OPENVRML_LIBRARY JPEG_LIBRARY PNG_LIBRARY ZLIB_LIBRARY)
    SET(TARGET_EXTERNAL_LIBRARIES Ws2_32.lib)
ELSE(WIN32)
    INCLUDE_DIRECTORIES( ${OPENVRML_INCLUDE_DIR} ${OPENVRML_INCLUDE_DIR}/openvrml)
ENDIF(WIN32)


