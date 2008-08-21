##############################################################################
# CMake build system for ARToolKit 2.7.x                                     #
#                                                                            #
# (c) Copyright 2007-2008, HITLabNZ, Hartmut Seichter                        #
##############################################################################

set(pattern_files
	blankPatt.gif
	calib_cpara.pdf
	calib_dist.pdf
	calibPatt.pdf
	pattHiro.pdf
	pattKanji.pdf
	pattMulti.pdf
	pattSample1.pdf
	pattSample2.pdf
	)

install(FILES ${pattern_files}	
	DESTINATION ${ARTOOLKIT_SHARE_FOLDER}/patterns
	)
