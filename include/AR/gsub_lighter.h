/*
 *	gsub_lite.h
 *
 *	Graphics Subroutines (Lite) for ARToolKit.
 *
 *	Copyright (c) 2003-2007 Philip Lamb (PRL) phil@eden.net.nz. All rights reserved.
 *	
 *	Rev		Date		Who		Changes
 *  2.7.0   2003-08-13  PRL     Complete rewrite to ARToolKit-2.65 gsub.c API.
 *  2.7.1   2004-03-03  PRL		Avoid defining BOOL if already defined
 *	2.7.1	2004-03-03	PRL		Don't enable lighting if it was not enabled.
 *	2.7.2	2004-04-27	PRL		Added headerdoc markup. See http://developer.apple.com/darwin/projects/headerdoc/
 *	2.7.3	2004-07-02	PRL		Much more object-orientated through use of ARGL_CONTEXT_SETTINGS type.
 *	2.7.4	2004-07-14	PRL		Added gluCheckExtension hack for GLU versions pre-1.3.
 *	2.7.5	2004-07-15	PRL		Added arglDispImageStateful(); removed extraneous glPixelStorei(GL_UNPACK_IMAGE_HEIGHT,...) calls.
 *	2.7.6	2005-02-18	PRL		Go back to using int rather than BOOL, to avoid conflict with Objective-C.
 *	2.7.7	2005-07-26	PRL		Added cleanup routines for texture stuff.
 *	2.7.8	2005-07-29	PRL		Added distortion compensation enabling/disabling.
 *	2.7.9	2005-08-15	PRL		Added complete support for runtime selection of pixel format and rectangle/power-of-2 textures.
 *	2.8.0	2006-04-04	PRL		Move pixel format constants into toolkit global namespace (in config.h).
 *	2.8.1	2006-04-06	PRL		Move arglDrawMode, arglTexmapMode, arglTexRectangle out of global variables.
 *  2.8.2   2006-06-12  PRL		More stringent runtime GL caps checking. Fix zoom for DRAWPIXELS mode.
 *
 */
/*
 * 
 * This file is part of ARToolKit.
 * 
 * ARToolKit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * ARToolKit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ARToolKit; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */

/**
	\file gsub_lite.h
	\brief A collection of useful OpenGL routines for ARToolKit.
	\cond GSUB_LITE
	\htmlinclude gsub_lite/index.html
*/

/*!
	@header gsub_lite
	@abstract A collection of useful OpenGL routines for ARToolKit.
	@discussion
		Sample code for example usage of gsub_lite is included with
		ARToolKit, in the directory &lt;AR/examples/simpleLite&gt;.
		
		gsub_lite is the preferred means for drawing camera video 
		images acquired from ARToolKit's video libraries. It includes
		optimized texture handling, and a variety of flexible drawing
		options.
 
		gsub_lite also provides utility functions for setting the
		OpenGL viewing frustum and camera position based on ARToolKit-
		camera parameters and marker positions.
 
		gsub_lite does not depend on GLUT, or indeed, any particular
		window or event handling system. It is therefore well suited
		to use in applications which have their own window and event
		handling code.
 
		gsub_lite v2.7 is intended as a replacement for gsub from
		ARToolKit 2.65, by Mark Billinghurst (MB) & Hirokazu Kato (HK),
		with the following additional functionality:
		<ul>
			<li> Support for true stereo and multiple displays through removal
			of most dependencies on global variables.
			<li> Prepared library for thread-safety by removing global variables.
			<li> Optimised texturing, particularly for Mac OS X platform.
			<li> Added arglCameraFrustum to replace argDraw3dCamera() function.
			<li> Renamed argConvGlpara() to arglCameraView() to more accurately
			represent its functionality.
			<li> Correctly handle textures with non-RGBA handling.
			<li> Library setup and cleanup functions.
			<li> Version numbering.
		</ul>
		It does however lack the following functionality from the original gsub
		library:
		<ul>
			<li> GLUT event handling.
			<li> Sub-window ("MINIWIN") and half-size drawing.
			<li> HMD support for stereo via stencil.
		</ul>
			
		This file is part of ARToolKit.
		
		ARToolKit is free software; you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation; either version 2 of the License, or
		(at your option) any later version.
		
		ARToolKit is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
		GNU General Public License for more details.
		
		You should have received a copy of the GNU General Public License
		along with ARToolKit; if not, write to the Free Software
		Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
	@copyright 2003-2007 Philip Lamb
	@updated 2006-05-23
 */

#ifndef __gsub_lighter_h__
#define __gsub_lighter_h__

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
//	Public includes.
// ============================================================================

#include <AR/config.h>
#include <AR/ar.h>		// ARUint8, AR_PIXEL_FORMAT, arDebug, arImage.
#include <AR/param.h>	// ARParam, arParamDecompMat(), arParamObserv2Ideal()


// ============================================================================
//	Public functions.
// ============================================================================
	
/*!
    @function
    @abstract Create an OpenGL perspective projection matrix.
    @discussion
		Use this function to create a matrix suitable for passing to OpenGL
		to set the viewing projection.
    @param cparam Pointer to a set of ARToolKit camera parameters for the
		current video source.
	@param focalmax The maximum distance at which geometry will be rendered.
		Any geometry further away from the camera than this distance will be clipped
		and will not be appear in a rendered frame. Thus, this value should be
		set high enough to avoid clipping of any geometry you care about. However,
		the precision of the depth buffer is correlated with the ratio of focalmin
		to focalmax, thus you should not set focalmax any higher than it needs to be.
		This value should be specified in the same units as your OpenGL drawing.
	@param focalmin The minimum distance at which geometry will be rendered.
		Any geometry closer to the camera than this distance will be clipped
		and will not be appear in a rendered frame. Thus, this value should be
		set low enough to avoid clipping of any geometry you care about. However,
		the precision of the depth buffer is correlated with the ratio of focalmin
		to focalmax, thus you should not set focalmin any lower than it needs to be.
		Additionally, geometry viewed in a stereo projections that is too close to
		camera is difficult and tiring to view, so if you are rendering stereo
		perspectives you should set this value no lower than the near-point of
		the eyes. The near point in humans varies, but usually lies between 0.1 m
		0.3 m. This value should be specified in the same units as your OpenGL drawing.
	@param m_projection Pointer to a array of 16 GLdoubles, which will be filled
		out with a projection matrix suitable for passing to OpenGL. The matrix
		is specified in column major order.
	@availability First appeared in ARToolKit 2.68.
*/
void arglCameraFrustum(const ARParam *cparam, const double focalmin, const double focalmax, double m_projection[16]);

/*!
    @function 
    @abstract   (description)
    @discussion (description)
    @param      (name) (description)
    @result     (description)
*/
void arglCameraFrustumRH(const ARParam *cparam, const double focalmin, const double focalmax, double m_projection[16]);

/*!
    @function
    @abstract Create an OpenGL viewing transformation matrix.
	@discussion
		Use this function to create a matrix suitable for passing to OpenGL
		to set the viewing transformation of the virtual camera.
	@param para Pointer to 3x4 matrix array of doubles which specify the
		position of an ARToolKit marker, as returned by arGetTransMat().
	@param m_modelview Pointer to a array of 16 GLdoubles, which will be filled
		out with a modelview matrix suitable for passing to OpenGL. The matrix
		is specified in column major order.
	@param scale Specifies a scaling between ARToolKit's
		units (usually millimeters) and OpenGL's coordinate system units.
		What you pass for the scalefactor parameter depends on what units you
		want to do your OpenGL drawing in. If you use a scalefactor of 1.0, then
		1.0 OpenGL unit will equal 1.0 millimetre (ARToolKit's default units).
		To use different OpenGL units, e.g. metres, then you would pass 0.001.
 	@availability First appeared in ARToolKit 2.68.
*/
void arglCameraView(const double para[3][4], double *m_modelview, const double scale);

/*!
    @function 
    @abstract   (description)
    @discussion (description)
    @param      (name) (description)
    @result     (description)
*/
void arglCameraViewRH(const double para[3][4], double m_modelview[16], const double scale);


#ifdef __cplusplus
}
#endif

/**
	\endcond
 */

#endif /* !__gsub_lite_h__ */
