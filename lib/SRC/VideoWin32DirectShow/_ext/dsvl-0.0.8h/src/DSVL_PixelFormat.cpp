/* ========================================================================
*  PROJECT: DirectShow Video Processing Library (DSVL)
*  Version: 0.0.8 (05/13/2005)
*  ========================================================================
*  Author:  Thomas Pintaric, Vienna University of Technology
*  Contact: pintaric@ims.tuwien.ac.at http://ims.tuwien.ac.at/~thomas
*  =======================================================================
* 
*  Copyright (C) 2005  Vienna University of Technology
* 
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License
*  as published by the Free Software Foundation; either version 2
*  of the License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*  For further information please contact Thomas Pintaric under
*  <pintaric@ims.tuwien.ac.at> or write to Thomas Pintaric,
*  Vienna University of Technology, Favoritenstr. 9-11/E188/2, A-1040
*  Vienna, Austria.
* ========================================================================*/

//#include <streams.h>

#include <atlbase.h>
#include <dshow.h>
#include <qedit.h>

#include <GL\gl.h>
#include "DSVL_PixelFormat.h"

const char PIXELFORMAT_names[PIXELFORMAT_ENUM_MAX][32] =
{ "PIXELFORMAT_UNKNOWN",
  "PIXELFORMAT_UYVY",
  "PIXELFORMAT_YUY2",
  "PIXELFORMAT_RGB565",
  "PIXELFORMAT_RGB555",
  "PIXELFORMAT_RGB24",
  "PIXELFORMAT_RGB32",
  "PIXELFORMAT_INVALID",
  "PIXELFORMAT_QUERY" };

int PXBitsPerPixel(PIXELFORMAT format)
{
	switch(format)
	{
	case PIXELFORMAT_RGB565:return(16);
	case PIXELFORMAT_RGB555:return(16);
	case PIXELFORMAT_RGB24: return(24);
	case PIXELFORMAT_RGB32: return(32);
	};
	return(0);
}

#ifndef GL_BGR
#define GL_BGR GL_BGR_EXT
#endif

#ifndef GL_BGRA
#define GL_BGRA GL_BGRA_EXT
#endif

WORD PXtoOpenGL(PIXELFORMAT format, bool bWIN32format)
{
	switch(format)
	{
	case PIXELFORMAT_RGB24: return(bWIN32format ? GL_BGR  : GL_RGB);
	case PIXELFORMAT_RGB32: return(bWIN32format ? GL_BGRA : GL_RGBA);
	};
	return(0);
}


GUID PXtoMEDIASUBTYPE(PIXELFORMAT format)
{
	switch(format)
	{
	case PIXELFORMAT_UYVY:  return(MEDIASUBTYPE_UYVY);
	case PIXELFORMAT_YUY2 : return(MEDIASUBTYPE_YUY2);
	case PIXELFORMAT_RGB565:return(MEDIASUBTYPE_RGB565);
	case PIXELFORMAT_RGB555:return(MEDIASUBTYPE_RGB555);
	case PIXELFORMAT_RGB24: return(MEDIASUBTYPE_RGB24);
	case PIXELFORMAT_RGB32: return(MEDIASUBTYPE_RGB32);
	};
	return(CLSID_NULL);
}


PIXELFORMAT MEDIASUBTYPEtoPX(GUID format)
{
	if(format == MEDIASUBTYPE_UYVY)   return(PIXELFORMAT_UYVY);
	if(format == MEDIASUBTYPE_YUY2)   return(PIXELFORMAT_YUY2);
	if(format == MEDIASUBTYPE_RGB565) return(PIXELFORMAT_RGB565);
	if(format == MEDIASUBTYPE_RGB555) return(PIXELFORMAT_RGB555);
	if(format == MEDIASUBTYPE_RGB24)  return(PIXELFORMAT_RGB24);
	if(format == MEDIASUBTYPE_RGB32)  return(PIXELFORMAT_RGB32);
	return(PIXELFORMAT_UNKNOWN);	
}

PIXELFORMAT OpenGLtoPX(WORD format)
{
	switch(format)
	{
	case GL_BGR:	return(PIXELFORMAT_RGB24);
	case GL_BGRA:	return(PIXELFORMAT_RGB32);
	};

	return(PIXELFORMAT_UNKNOWN);
}

const char* PXtoString(PIXELFORMAT format)
{
	return(&(PIXELFORMAT_names[format][0]));
}

PIXELFORMAT StringToPX(char* formatName)
{
	for(unsigned int i=0; i<PIXELFORMAT_ENUM_MAX; i++)
		if(strcmp(formatName,PIXELFORMAT_names[i]) == 0) return((PIXELFORMAT)i);
	return(PIXELFORMAT_UNKNOWN);
}
