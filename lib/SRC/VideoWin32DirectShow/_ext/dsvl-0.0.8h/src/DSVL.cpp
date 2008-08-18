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

#include "DSVL.h"
#include "DSVL_GraphManager.h"

#define GM(void_ptr) (DSVL_GraphManager*)void_ptr

DSVL_VideoSource::DSVL_VideoSource()
{
	p_graphManager = GM(new DSVL_GraphManager());
}

DSVL_VideoSource::~DSVL_VideoSource()
{
	(GM(p_graphManager))->ReleaseGraph();
	delete GM(p_graphManager);
}

HRESULT DSVL_VideoSource::BuildGraphFromXMLString(char* xml_string)
{
	return((GM(p_graphManager))->BuildGraphFromXMLString(xml_string));
}

HRESULT DSVL_VideoSource::BuildGraphFromXMLFile(char* xml_filename)
{
	return((GM(p_graphManager))->BuildGraphFromXMLFile(xml_filename));
}

HRESULT DSVL_VideoSource::ReleaseGraph()
{
	return((GM(p_graphManager))->ReleaseGraph());
}

HRESULT DSVL_VideoSource::EnableMemoryBuffer(unsigned int _maxConcurrentClients,
											 unsigned int _allocatorBuffersPerClient)
{
	return((GM(p_graphManager))->EnableMemoryBuffer(_maxConcurrentClients,_allocatorBuffersPerClient));
}

HRESULT DSVL_VideoSource::DisableMemoryBuffer()
{
	return((GM(p_graphManager))->DisableMemoryBuffer());
}

bool DSVL_VideoSource::IsGraphInitialized()
{
	return((GM(p_graphManager))->IsGraphInitialized());
}

DWORD DSVL_VideoSource::WaitForNextSample(long dwMilliseconds)
{
	return((GM(p_graphManager))->WaitForNextSample(dwMilliseconds));
}

HRESULT DSVL_VideoSource::CheckoutMemoryBuffer(MemoryBufferHandle* pHandle, 
                                               BYTE** Buffer,
                                               unsigned int *Width,
                                               unsigned int *Height,
                                               PIXELFORMAT* PixelFormat,
                                               REFERENCE_TIME* Timestamp)
{
	return((GM(p_graphManager))->CheckoutMemoryBuffer(pHandle,Buffer,Width,Height,PixelFormat,Timestamp));
}

HRESULT DSVL_VideoSource::CheckinMemoryBuffer(MemoryBufferHandle Handle, bool ForceRelease)
{
	return((GM(p_graphManager))->CheckinMemoryBuffer(Handle, ForceRelease));
}

HRESULT DSVL_VideoSource::GetCurrentMediaFormat(LONG* frame_width,
                                                LONG *frame_height,
                                                double* frames_per_second,
                                                PIXELFORMAT* pixel_format)
{
	return((GM(p_graphManager))->GetCurrentMediaFormat(frame_width, frame_height, frames_per_second, pixel_format));
}

LONGLONG DSVL_VideoSource::GetCurrentTimestamp()
{
	return((GM(p_graphManager))->GetCurrentTimestamp());
}


HRESULT DSVL_VideoSource::Run()
{
	return((GM(p_graphManager))->Run());
}

HRESULT DSVL_VideoSource::Pause()
{
	return((GM(p_graphManager))->Pause());
}

HRESULT DSVL_VideoSource::Stop(bool forcedStop)
{
	return((GM(p_graphManager))->Stop(forcedStop));
}


