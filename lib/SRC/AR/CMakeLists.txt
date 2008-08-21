set(LIB_NAME AR)

include_directories(${CMAKE_SOURCE_DIR}/include)

configure_file(
	${CMAKE_SOURCE_DIR}/include/${LIB_NAME}/config.h.in
	${CMAKE_BINARY_DIR}/include/${LIB_NAME}/config.h
	)
	
set(SRCS 
	arDetectMarker.c
	arDetectMarker2.c
	arGetCode.c
	arGetMarkerInfo.c
	arGetTransMat.c
	arGetTransMat2.c
	arGetTransMat3.c
	arGetTransMatCont.c
	arLabeling.c
	arUtil.c
	mAlloc.c
	mAllocDup.c
	mAllocInv.c
	mAllocMul.c
	mAllocTrans.c
	mAllocUnit.c
	mDet.c
	mDisp.c
	mDup.c
	mFree.c
	mInv.c
	mMul.c
	mPCA.c
	mSelfInv.c
	mTrans.c
	mUnit.c
	paramChangeSize.c
	paramDecomp.c
	paramDisp.c
	paramDistortion.c
	paramFile.c
	paramGet.c
	vAlloc.c
	vDisp.c
	vFree.c
	vHouse.c
	vInnerP.c
	vTridiag.c
	)

set(HDRS ${HDRS} 
	${CMAKE_BINARY_DIR}/include/AR/config.h
	)
	
set(HDRS ${HDRS} ${CMAKE_SOURCE_DIR}/include/AR/ar.h)

add_library(${LIB_NAME} ${SRCS} ${HDRS})

artoolkit_install(${LIB_NAME})	
	
install(FILES ${HDRS}
	DESTINATION include/AR
	)

	
	
