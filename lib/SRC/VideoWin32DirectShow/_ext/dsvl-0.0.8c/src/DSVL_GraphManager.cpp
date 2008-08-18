/* ========================================================================
*  PROJECT: DirectShow Video Processing Library (DSVL)
*  Version: 0.0.8c (10/26/2005)
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


#include "DSVL_GraphManager.h"

#include "dshow.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <objbase.h>
#include <comutil.h>
#include <atlbase.h>
#include <process.h>

#include "tinyxml.h"
#include "DSVL_Helpers.h"

//HANDLE g_ESync = CreateEvent(NULL,0,0,_T("global sync event"));
//CCritSec g_CSec;

#define MAX_GUID_STRING_LENGTH 72

#ifdef _DEBUG
#define DsVideoLibFileName "DSVLd.dll"
#else
#define DsVideoLibFileName "DSVL.dll"
#endif


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

// -----------------------------------------------------------------------------------------------------------------
DS_MEDIA_FORMAT default_DS_MEDIA_FORMAT()
{
	DS_MEDIA_FORMAT mf;
	ZeroMemory(&mf, sizeof(DS_MEDIA_FORMAT));
	mf.inputDevice = default_VIDEO_INPUT_DEVICE;
	mf.pixel_format = default_PIXELFORMAT;
	mf.inputFlags = 0; // clear flags
	mf.defaultInputFlags = true;
	return(mf);
}

// -----------------------------------------------------------------------------------------------------------------
HRESULT MatchMediaTypes(IPin *pin, DS_MEDIA_FORMAT *mf, AM_MEDIA_TYPE *req_mt) 
// will overwrite req_mt with best match
{
	HRESULT hr;
	CComPtr<IEnumMediaTypes> enum_mt;
	pin->EnumMediaTypes(&enum_mt);
	enum_mt->Reset();
	AM_MEDIA_TYPE *p_mt = NULL;
	while(S_OK == (hr = enum_mt->Next(1,&p_mt,NULL)))
	{
		VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) p_mt->pbFormat;
		if( ((p_mt->subtype == mf->subtype) || (mf->subtype == GUID_NULL)) &&
			((pvi->bmiHeader.biHeight == mf->biHeight) || (mf->biHeight == 0)) &&
			((pvi->bmiHeader.biWidth  == mf->biWidth)  || (mf->biWidth == 0)) &&
			((avg2fps(pvi->AvgTimePerFrame,3) == RoundDouble(mf->frameRate,3)) || (mf->frameRate == 0.0))
			)
		{
			// found a match!
			CopyMediaType(req_mt,p_mt);
			DeleteMediaType(p_mt);
			return(S_OK);
		}
		else DeleteMediaType(p_mt);
	}

	return(E_FAIL);
}

// -----------------------------------------------------------------------------------------------------------------
const char* VideoInputDeviceToString(VIDEO_INPUT_DEVICE device)
{
	if(device > INVALID_INPUT_FILTER) return(NULL);
	else return(VIDEO_INPUT_DEVICE_names[device]);
}

// -----------------------------------------------------------------------------------------------------------------
VIDEO_INPUT_DEVICE StringVideoInputDevice(char *device_str)
{
	for(unsigned int i=0; i<INVALID_INPUT_FILTER; i++)
		if(strcmp(device_str,VIDEO_INPUT_DEVICE_names[i]) == 0) return((VIDEO_INPUT_DEVICE)i);
	return(INVALID_INPUT_FILTER);
}


// -----------------------------------------------------------------------------------------------------------------




//###############################################################################################

DSVL_GraphManager::DSVL_GraphManager() :
	  captureGraphBuilder(NULL),
	  graphBuilder(NULL),
	  mediaControl(NULL),
	  mediaEvent(NULL),
	  mediaSeeking(NULL),
	  cameraControl(NULL),
	  droppedFrames(NULL),
	  videoControl(NULL),
	  videoProcAmp(NULL),
	  sourceFilter(NULL),
	  decoderFilter(NULL),
	  rendererFilter(NULL),
	  grabberFilter(NULL),
	  capturePin(NULL),
	  m_ESync(NULL),
	  m_ESyncName("SyncEvent")
{

	m_ESync = CreateEvent(NULL,TRUE,0,_T("SyncEvent"));

	media_format = default_DS_MEDIA_FORMAT();
	media_format.subtype = GUID_NULL;

	current_timestamp = 0;
	sample_counter = 0;

#ifdef _DEBUG
	/*
	DbgInitialise((struct HINSTANCE__ *) GetModuleHandle(DsVideoLibFileName));
	DbgSetModuleLevel(LOG_TRACE,5);
	DbgSetModuleLevel(LOG_MEMORY,5);
	DbgSetModuleLevel(LOG_ERROR,5);
	DbgSetModuleLevel(LOG_TIMING,5);
	DbgSetModuleLevel(LOG_LOCKING,5);
	*/
#endif

	m_bGraphIsInitialized = false;
}

// -----------------------------------------------------------------------------------------------------------------
DSVL_GraphManager::~DSVL_GraphManager()
{
  HRESULT hr;
  hr = ReleaseGraph();

  #ifdef _DEBUG
	//DbgOutString("*** DbgDumpObjectRegister:\n");
	//DbgDumpObjectRegister();
  #endif
}

// ----------------------------------------------------------------------------------------------------------
HRESULT DSVL_GraphManager::GetCurrentMediaFormatEx(DS_MEDIA_FORMAT *mf)
{ 
	CopyMemory(mf,&media_format,sizeof(DS_MEDIA_FORMAT));
	return(S_OK);
}

// ----------------------------------------------------------------------------------------------------------
HRESULT DSVL_GraphManager::GetCurrentMediaFormat(LONG* frame_width,
												LONG *frame_height,
												double* frames_per_second,
												PIXELFORMAT* pixel_format)
{
	DS_MEDIA_FORMAT mf;
	HRESULT hr = GetCurrentMediaFormatEx(&mf);
	if(FAILED(hr)) return(hr);
	if(frame_width != NULL) *frame_width = mf.biWidth;
	if(frame_height != NULL) *frame_height = mf.biHeight;
	if(frames_per_second != NULL) *frames_per_second = mf.frameRate;
	if(pixel_format != NULL) *pixel_format = mf.pixel_format;
	return(S_OK);
}

// ----------------------------------------------------------------------------------------------------------
HRESULT DSVL_GraphManager::EnableMemoryBuffer(unsigned int _maxConcurrentClients,
						   unsigned int _allocatorBuffersPerClient)
{
	CAutoLock cObjectLock(&m_CSec);
	HRESULT hr;
    
	if(_allocatorBuffersPerClient < MIN_ALLOCATOR_BUFFERS_PER_CLIENT)
		_allocatorBuffersPerClient = MIN_ALLOCATOR_BUFFERS_PER_CLIENT;
	if(_maxConcurrentClients <= 0) _maxConcurrentClients = 1;

	m_currentAllocatorBuffers = _maxConcurrentClients * _allocatorBuffersPerClient;

	// ------
	CComPtr<IPin> sgPin = NULL;
	if(FAILED(hr=getPin(grabberFilter,PINDIR_INPUT,1,sgPin))) return(hr); 
	CComPtr<IMemAllocator> pAllocator = NULL;

	CComPtr<IMemInputPin> sgmiPin = NULL; 
	hr = sgPin->QueryInterface(IID_IMemInputPin, (void**)&sgmiPin);
	if (FAILED(hr)) return(hr);

	if(FAILED(hr = sgmiPin->GetAllocator(&pAllocator))) return(hr);
	if(FAILED(hr = pAllocator->Decommit())) return(hr);
	ALLOCATOR_PROPERTIES requestedProperties;
	ALLOCATOR_PROPERTIES actualProperties;
	pAllocator->GetProperties(&requestedProperties);
	if(requestedProperties.cBuffers != m_currentAllocatorBuffers)
		requestedProperties.cBuffers = m_currentAllocatorBuffers;
	hr = pAllocator->SetProperties(&requestedProperties,&actualProperties);
	// ------

	sampleGrabber->SetBufferSamples(FALSE);
	sampleGrabber->SetOneShot(FALSE);

	CComPtr<ISampleGrabberCB> custom_sgCB;
	hr = QueryInterface(IID_ISampleGrabberCB, (void**)&custom_sgCB);
	if(FAILED(hr)) return(hr);
	hr = sampleGrabber->SetCallback(custom_sgCB,0);
	return(hr);
}
// ----------------------------------------------------------------------------------------------------------
HRESULT DSVL_GraphManager::DisableMemoryBuffer()
{
	CAutoLock cObjectLock(&m_CSec);

	if(mb.size() > 0)
	{
		std::map<unsigned long, MemoryBufferEntry>::iterator iter = mb.begin();
		while(iter != mb.end())
		{
			// ignore use_count
			(*iter).second.media_sample->Release();
			std::map<unsigned long, MemoryBufferEntry>::iterator iter2 =
				mb.erase(iter);
			iter = iter2;
		}
	}

	m_currentAllocatorBuffers = 0;
	return(sampleGrabber->SetCallback(NULL,0));
}
// ----------------------------------------------------------------------------------------------------------

DWORD WINAPI TShowFilterProperties(LPVOID lpParameter)
{
	return(DisplayFilterProperties((IBaseFilter*)lpParameter));
}
// ----------------------------------------------------------------------------------------------------------
DWORD WINAPI TShowPinProperties(LPVOID lpParameter)
{
	return(DisplayPinProperties((IPin*)lpParameter));
}
// ----------------------------------------------------------------------------------------------------------
HRESULT DSVL_GraphManager::ShowFilterProperties(HWND hWnd)
{
	DWORD dwThreadID;
	HANDLE hFPropThread = CreateThread(NULL,0,TShowFilterProperties,(LPVOID)sourceFilter,0,&dwThreadID);
	if(hFPropThread == NULL) return(E_FAIL);
	else return(S_OK);
}
// ----------------------------------------------------------------------------------------------------------
HRESULT DSVL_GraphManager::ShowPinProperties(HWND hWnd)
{
	DWORD dwThreadID;
	HANDLE hPPropThread = CreateThread(NULL,0,TShowPinProperties,(LPVOID)capturePin,0,&dwThreadID);
	if(hPPropThread == NULL) return(E_FAIL);
	else return(S_OK);
}
// ----------------------------------------------------------------------------------------------------------
HRESULT DSVL_GraphManager::GetCameraParameterRange(CP_INTERFACE interface_type,  
									long property,
									long *pMin,
									long *pMax,
									long *pSteppingDelta,
									long *pDefault,
									long *pCapsFlags)
{
	switch(interface_type)
	{
	case(CP_CameraControl): if(!cameraControl) return(E_INVALIDARG);
							return(cameraControl->GetRange(property,pMin,pMax,pSteppingDelta,pDefault,pCapsFlags));
		break;
	case(CP_VideoProcAmp):  if(!videoProcAmp) return(E_INVALIDARG);
							return(videoProcAmp->GetRange(property,pMin,pMax,pSteppingDelta,pDefault,pCapsFlags));
		break;
	}

	// unknown interface_type
	return(E_INVALIDARG);
}

// ----------------------------------------------------------------------------------------------------------
HRESULT DSVL_GraphManager::GetCameraParameter(CP_INTERFACE interface_type, long Property, long *lValue, bool *bAuto)
{
	HRESULT hr;
	long Flags = 0;
	hr = GetCameraParameter(interface_type,Property,lValue,&Flags);
	(*bAuto) = (Flags == GetCameraPropertyAUTOFlag(interface_type, true));
	return(hr);
}

HRESULT DSVL_GraphManager::GetCameraParameter(CP_INTERFACE interface_type, long Property, long *lValue, long *Flags)
{
	switch(interface_type)
	{
	case(CP_CameraControl): if(!cameraControl) return(E_INVALIDARG);
							return(cameraControl->Get(Property,lValue,Flags));
		break;
	case(CP_VideoProcAmp):  if(!videoProcAmp) return(E_INVALIDARG);
							return(videoProcAmp->Get(Property,lValue,Flags));
		break;
	}

	// unknown interface_type
	return(E_INVALIDARG);
}

// ----------------------------------------------------------------------------------------------------------
HRESULT DSVL_GraphManager::SetCameraParameter(CP_INTERFACE interface_type, long Property, long lValue, bool bAuto)
{
	return(SetCameraParameter(interface_type,Property,lValue,GetCameraPropertyAUTOFlag(interface_type, bAuto)));
}

// ----------------------------------------------------------------------------------------------------------
HRESULT DSVL_GraphManager::SetCameraParameter(CP_INTERFACE interface_type, long Property, long lValue, long Flags)
{
	switch(interface_type)
	{
	case(CP_CameraControl): if(!cameraControl) return(E_INVALIDARG);
							return(cameraControl->Set(Property,lValue,Flags));
		break;
	case(CP_VideoProcAmp):  if(!videoProcAmp) return(E_INVALIDARG);
							return(videoProcAmp->Set(Property,lValue,Flags));
		break;
	}

	// unknown interface_type
	return(E_INVALIDARG);
}
// ----------------------------------------------------------------------------------------------------------

long DSVL_GraphManager::GetCameraPropertyAUTOFlag(CP_INTERFACE interface_type, bool bAUTO)
{
	switch(interface_type)
	{
	case(CP_CameraControl): return(bAUTO ? CameraControl_Flags_Auto : CameraControl_Flags_Manual);
		break;
	case(CP_VideoProcAmp):  return(bAUTO ? VideoProcAmp_Flags_Auto : VideoProcAmp_Flags_Manual);
		break;
	}
#ifdef _DEBUG
	ASSERT(false);
#endif
	return(0x01);
}

// ----------------------------------------------------------------------------------------------------------
HRESULT DSVL_GraphManager::SetCameraParameterToDefault(CP_INTERFACE interface_type, long Property, bool bAuto)
{
	long min;
	long max;
	long steppingDelta;
	long defaultValue;
	long capsFlags;

	HRESULT hr;
	if(FAILED(hr = GetCameraParameterRange(interface_type,Property,&min,&max,&steppingDelta,&defaultValue,&capsFlags)))
		return(hr);
	if(FAILED(hr = SetCameraParameter(interface_type,Property,defaultValue,GetCameraPropertyAUTOFlag(interface_type,bAuto))))
		return(hr);

	return(S_OK);
}

// ----------------------------------------------------------------------------------------------------------
HRESULT DSVL_GraphManager::ResetCameraParameters(bool bAuto)
// bAuto: indicates if the property should be controlled automatically
{
	int s = 0;
	if(cameraControl)
	{
		s += (int)SUCCEEDED(SetCameraParameterToDefault(CP_CameraControl, CameraControl_Pan, bAuto));
		s += (int)SUCCEEDED(SetCameraParameterToDefault(CP_CameraControl, CameraControl_Roll, bAuto));
		s += (int)SUCCEEDED(SetCameraParameterToDefault(CP_CameraControl, CameraControl_Tilt, bAuto));
		s += (int)SUCCEEDED(SetCameraParameterToDefault(CP_CameraControl, CameraControl_Zoom, bAuto));
		s += (int)SUCCEEDED(SetCameraParameterToDefault(CP_CameraControl, CameraControl_Exposure, bAuto));
		s += (int)SUCCEEDED(SetCameraParameterToDefault(CP_CameraControl, CameraControl_Iris, bAuto));
		s += (int)SUCCEEDED(SetCameraParameterToDefault(CP_CameraControl, CameraControl_Focus, bAuto));
		// impressive, but show me a camera that supports all of these options! :(
	}
	if(videoProcAmp)
	{
		s += (int)SUCCEEDED(SetCameraParameterToDefault(CP_VideoProcAmp, VideoProcAmp_Brightness, bAuto));
		s += (int)SUCCEEDED(SetCameraParameterToDefault(CP_VideoProcAmp, VideoProcAmp_Contrast, bAuto));
		s += (int)SUCCEEDED(SetCameraParameterToDefault(CP_VideoProcAmp, VideoProcAmp_Hue, bAuto));
		s += (int)SUCCEEDED(SetCameraParameterToDefault(CP_VideoProcAmp, VideoProcAmp_Saturation, bAuto));
		s += (int)SUCCEEDED(SetCameraParameterToDefault(CP_VideoProcAmp, VideoProcAmp_Sharpness, bAuto));
		s += (int)SUCCEEDED(SetCameraParameterToDefault(CP_VideoProcAmp, VideoProcAmp_Gamma, bAuto));
		s += (int)SUCCEEDED(SetCameraParameterToDefault(CP_VideoProcAmp, VideoProcAmp_ColorEnable, bAuto));
		s += (int)SUCCEEDED(SetCameraParameterToDefault(CP_VideoProcAmp, VideoProcAmp_WhiteBalance, bAuto));
		s += (int)SUCCEEDED(SetCameraParameterToDefault(CP_VideoProcAmp, VideoProcAmp_BacklightCompensation, bAuto));
		s += (int)SUCCEEDED(SetCameraParameterToDefault(CP_VideoProcAmp, VideoProcAmp_Gain, bAuto));
	}
	
	if(s==0) return(E_FAIL);
	else return(S_OK);
}
// ----------------------------------------------------------------------------------------------------------

HRESULT DSVL_GraphManager::GetCameraParameterN(CP_INTERFACE interface_type, long Property, double *dValue)
{
	HRESULT hr;
	long oldValue;
	long oldFlags;
	long min;
	long max;
	long steppingDelta;
	long defaultValue;
	long capsFlags;

	if(FAILED(hr = GetCameraParameter(interface_type,Property,&oldValue, &oldFlags))) return(hr);
	if(FAILED(hr = GetCameraParameterRange(interface_type,Property,&min,&max,&steppingDelta,&defaultValue,&capsFlags)))
		return(hr);

	(*dValue) = ((double)(oldValue-min))/((double)(max-min));
	return(S_OK);
}
// ----------------------------------------------------------------------------------------------------------
HRESULT DSVL_GraphManager::SetCameraParameterN(CP_INTERFACE interface_type, long Property, double dValue)
{
	HRESULT hr;
	long oldValue;
	long oldFlags;
	long min;
	long max;
	long steppingDelta;
	long defaultValue;
	long capsFlags;

	if(FAILED(hr = GetCameraParameter(interface_type,Property,&oldValue, &oldFlags))) return(hr);
	if(FAILED(hr = GetCameraParameterRange(interface_type,Property,&min,&max,&steppingDelta,&defaultValue,&capsFlags)))
		return(hr);

	long newValue = min + (long)((dValue*((double)(max-min))) / (double)steppingDelta) * steppingDelta;
	if(newValue == oldValue) return(S_FALSE);
	return(SetCameraParameter(interface_type,Property,newValue,GetCameraPropertyAUTOFlag(interface_type,false)));
}

// ----------------------------------------------------------------------------------------------------------
HRESULT DSVL_GraphManager::Run()
{
	HRESULT hr;
	if(FAILED(hr =	mediaControl->Run())) return(hr);
	else return(S_OK);
}

// ----------------------------------------------------------------------------------------------------------

HRESULT DSVL_GraphManager::Stop(bool forcedStop)
{
	HRESULT hr;

	if(FAILED(hr =	(forcedStop ? mediaControl->Stop() : mediaControl->StopWhenReady()))) return(hr);
	else return(S_OK);
}

// ----------------------------------------------------------------------------------------------------------

HRESULT DSVL_GraphManager::Pause()
{
	HRESULT hr;
	if(FAILED(hr =	mediaControl->Pause())) return(hr);
	else return(S_OK);
}

// ----------------------------------------------------------------------------------------------------------

HRESULT DSVL_GraphManager::GetCurrentTimestamp(REFERENCE_TIME *Timestamp)
{
	CAutoLock cObjectLock(&m_CSec);
	*Timestamp = current_timestamp;
	return(S_OK);
}

REFERENCE_TIME DSVL_GraphManager::GetCurrentTimestamp()
{
	CAutoLock cObjectLock(&m_CSec);
	return(current_timestamp);
}

// ----------------------------------------------------------------------------------------------------------

DWORD DSVL_GraphManager::WaitForNextSample(long dwMilliseconds)
{
	MSG	uMsg;
	memset(&uMsg,0,sizeof(uMsg));
	bool b_ExitMainLoop = false;

	do
	{
		DWORD dw = MsgWaitForMultipleObjectsEx(1,&m_ESync,dwMilliseconds,QS_ALLEVENTS,0);
		if(dw == WAIT_OBJECT_0 || dw == WAIT_TIMEOUT)
		{
			ResetEvent(m_ESync);
			return(dw);
		}
		else while( PeekMessage(&uMsg, NULL, 0, 0, PM_NOREMOVE ) )
		{
			if( !GetMessage(&uMsg, NULL, 0, 0 ) )
			{
				b_ExitMainLoop = true;
			}
			else
			{
				TranslateMessage(&uMsg);
				DispatchMessage(&uMsg);
			}
		}
	} while(!b_ExitMainLoop);

	return(WAIT_TIMEOUT); // ...whatever
}


HRESULT WINAPI DSVL_GraphManager::SampleCB(double SampleTime, IMediaSample *pSample)
{
	CAutoLock cObjectLock(&m_CSec);
	if(mb.size() > 0) // constantly clean up (mb)
	{
		std::map<unsigned long, MemoryBufferEntry>::iterator iter = mb.begin();
		while(iter != mb.end())
		{
			if((*iter).second.use_count == 0)
			{
				(*iter).second.media_sample->Release();
				std::map<unsigned long, MemoryBufferEntry>::iterator iter2 =
					mb.erase(iter);
				iter = iter2;
			}
			else
			{
				iter++;
			}
		}
	}

	HRESULT hr;
	REFERENCE_TIME t_start, t_end;
	hr = pSample->GetMediaTime(&t_start,&t_end);   // we just care about the start
	current_timestamp = t_start;
	sample_counter++;
	MemoryBufferEntry mb_entry;
	mb_entry.use_count = 0;
	mb_entry.timestamp = current_timestamp;
	
	mb_entry.media_sample = pSample;
	pSample->AddRef();

	mb.insert(pair<unsigned long, MemoryBufferEntry>
		(unsigned long(sample_counter),mb_entry));

	// flipping
	if(media_format.flipH || media_format.flipV)
	{
		BYTE *pBuffer;
		if(SUCCEEDED(pSample->GetPointer(&pBuffer)))
			FlipImageRGB32(pBuffer,media_format.biWidth,media_format.biHeight,media_format.flipH,media_format.flipV);
	}
	// --------
	SetEvent(m_ESync);

	return(S_OK);
}

HRESULT WINAPI DSVL_GraphManager::BufferCB(double sampleTimeSec, BYTE* bufferPtr, long bufferLength)
{
	return(E_NOTIMPL);
}


HRESULT WINAPI DSVL_GraphManager::QueryInterface(REFIID iid, void** ppvObject )
{
	// Return requested interface
	if (IID_IUnknown == iid)
	{
		*ppvObject = dynamic_cast<IUnknown*>( this );
	}
	else if (IID_ISampleGrabberCB == iid)
	{
		// Sample grabber callback object
		*ppvObject = dynamic_cast<ISampleGrabberCB*>( this );
	}
	else
	{     
		// No interface for requested iid - return error.
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	// inc reference count
	this->AddRef();
	return S_OK;
}

ULONG WINAPI DSVL_GraphManager::AddRef()
{
	return(fRefCount++);
}

ULONG WINAPI DSVL_GraphManager::Release()
{
	if (fRefCount > 0) fRefCount--;
	return fRefCount;
}


HRESULT DSVL_GraphManager::CheckoutMemoryBuffer(MemoryBufferHandle* pHandle,
											BYTE** Buffer,
											unsigned int *Width,
											unsigned int *Height,
											PIXELFORMAT* PixelFormat,
											REFERENCE_TIME* Timestamp)
{
	CAutoLock cObjectLock(&m_CSec);
	HRESULT hr;
	std::map<unsigned long, MemoryBufferEntry>::iterator iter;
	iter = mb.find(sample_counter);

	if(iter == mb.end()) // i.e. (sample_counter == 0)
		return(E_INVALIDARG); // no buffer in memory


	(*iter).second.use_count++;
	if(Buffer != NULL)
		(*iter).second.media_sample->GetPointer(Buffer);
	if(Width != NULL)
		*Width = media_format.biWidth;
	if(Height != NULL)
		*Height = media_format.biHeight;
	if(PixelFormat != NULL)
		*PixelFormat = media_format.pixel_format;
	if(Timestamp != NULL)
		*Timestamp = (*iter).second.timestamp;

	pHandle->n = (*iter).first;
	pHandle->t = (*iter).second.timestamp;
	return(S_OK);
}

// ----------------------------------------------------------------------------------------------------------
HRESULT DSVL_GraphManager::CheckinMemoryBuffer(MemoryBufferHandle Handle, bool ForceRelease)
{
	CAutoLock cObjectLock(&m_CSec);
	std::map<unsigned long, MemoryBufferEntry>::iterator iter;
	iter = mb.find(Handle.n);
	ASSERT(iter != mb.end());

	if(ForceRelease) (*iter).second.use_count = 0;
	else
	{
		if((*iter).second.use_count > 0)
			(*iter).second.use_count--;
	}
	return(S_OK);
}


HRESULT DSVL_GraphManager::ReleaseGraph()
{
	CAutoLock cObjectLock(&m_CSec);
	if(!m_bGraphIsInitialized) return(E_FAIL); // call BuildGraph() first!
	HRESULT hr = mediaControl->Stop();

	// Enumerate the filters in the graph.
	IEnumFilters *pEnum = NULL;
	hr = graphBuilder->EnumFilters(&pEnum);
	if (SUCCEEDED(hr))
	{
		IBaseFilter *pFilter = NULL;
		while (S_OK == pEnum->Next(1, &pFilter, NULL))
		{
			FILTER_INFO f_info;
			HRESULT hr_f = pFilter->QueryFilterInfo(&f_info);

			// Remove the filter.
			hr = graphBuilder->RemoveFilter(pFilter);
#ifdef _DEBUG
			_ASSERT(SUCCEEDED(hr));
#endif

			if(SUCCEEDED(hr_f))
			{
				f_info.pGraph->Release();
			}
			else
			{
#ifdef _DEBUG
				_ASSERT(FALSE);
#endif
			}

			// Reset the enumerator.
			pEnum->Reset();
			pFilter->Release();
		}
		pEnum->Release();
	}

	mediaControl.Release();
	graphBuilder.Release();

	if(captureGraphBuilder!=NULL) captureGraphBuilder.Release();
	if(mediaEvent!=NULL) mediaEvent.Release();
	if(mediaSeeking!=NULL) mediaSeeking.Release();
	if(cameraControl!=NULL) cameraControl.Release();
	if(droppedFrames!=NULL) droppedFrames.Release();
	if(videoControl!=NULL) videoControl.Release();
	if(videoProcAmp!=NULL) videoProcAmp.Release();
	if(sourceFilter!=NULL) sourceFilter.Release();
	if(capturePin!=NULL) capturePin.Release();
	if(decoderFilter!=NULL) decoderFilter.Release();
	if(rendererFilter!=NULL) rendererFilter.Release();
	if(grabberFilter!=NULL) grabberFilter.Release();
	if(sampleGrabber!=NULL) sampleGrabber.Release();


#ifdef _DEBUG
	RemoveFromRot(dwRegisterROT);
	dwRegisterROT = 0;
#endif

	m_bGraphIsInitialized = false;
	return(S_OK);

}


HRESULT DSVL_GraphManager::BuildGraphFromXMLString(char* xml_string)
{
	TiXmlDocument xmlDoc;
	xmlDoc.Parse(xml_string);
	if(xmlDoc.Error()) return(E_INVALIDARG);
	TiXmlHandle xmlHandle(&xmlDoc);
	return(BuildGraphFromXMLHandle(xmlHandle));
}

HRESULT DSVL_GraphManager::BuildGraphFromXMLFile(char* xml_filename)
{
	TiXmlDocument xmlDoc(xml_filename);
	if(!xmlDoc.LoadFile())
	{
		char path[MAX_PATH];
		char* path_offset = NULL;
		if(SearchPath(NULL,_bstr_t(xml_filename),NULL,MAX_PATH,path,&path_offset) > 0)
		{
			if(!xmlDoc.LoadFile(_bstr_t(path))) return(E_INVALIDARG);
		}
	}

	TiXmlHandle xmlHandle(&xmlDoc);
	return(BuildGraphFromXMLHandle(xmlHandle));
}





HRESULT DSVL_GraphManager::BuildGraphFromXMLHandle(TiXmlHandle xml_h)
{
	if(m_bGraphIsInitialized) return(E_FAIL); // call ReleaseGraph() first!

	DS_MEDIA_FORMAT mf = default_DS_MEDIA_FORMAT();
	mf.defaultInputFlags = false;
	mf.sourceFilterName = (LPWSTR) CoTaskMemAlloc(sizeof(wchar_t) * MAX_PATH);
	swprintf(mf.sourceFilterName, sizeof(wchar_t) * MAX_PATH,L"");
	//mf.ieee1394_id = (char*) CoTaskMemAlloc(sizeof(char) * MAX_PATH);
	//swprintf(mf.ieee1394_id, sizeof(wchar_t) * MAX_PATH,L"");

	TiXmlHandle doc_handle(xml_h);
	TiXmlElement* e_avi = doc_handle.FirstChild("dsvl_input").FirstChild("avi_file").Element();
	TiXmlElement* e_camera = doc_handle.FirstChild("dsvl_input").FirstChild("camera").Element();
	TiXmlElement* e_pixel_format = NULL;

	if(e_camera)
	{
		mf.inputDevice = WDM_VIDEO_CAPTURE_FILTER;

		if(e_camera->Attribute("device_name") != NULL)
		{
			swprintf(mf.sourceFilterName, sizeof(wchar_t) * MAX_PATH,_bstr_t(e_camera->Attribute("device_name")));
			mf.isDeviceName = true;
			mf.inputFlags |= WDM_MATCH_FILTER_NAME;
		}
		else if(e_camera->Attribute("friendly_name") != NULL)
		{
			swprintf(mf.sourceFilterName, sizeof(wchar_t) * MAX_PATH,_bstr_t(e_camera->Attribute("friendly_name")));
			mf.isDeviceName = false;
			mf.inputFlags |= WDM_MATCH_FILTER_NAME;
		}
		
		if((e_camera->Attribute("show_format_dialog") != NULL) &&
			(_strnicmp("true",_bstr_t(e_camera->Attribute("show_format_dialog")),strlen("true")) == 0))
			mf.inputFlags |= WDM_SHOW_FORMAT_DIALOG;

		if(e_camera->Attribute("ieee1394id") != NULL)
		{
			mf.ieee1394_id = (char*) CoTaskMemAlloc(MAX_PATH);
			sprintf(mf.ieee1394_id, _bstr_t(e_camera->Attribute("ieee1394id")));
			mf.inputFlags |= WDM_MATCH_IEEE1394_ID;
		}

		if(e_camera->Attribute("frame_width") != NULL)
		{
			mf.biWidth = strtol(_bstr_t(e_camera->Attribute("frame_width")),NULL,10);
			if(mf.biWidth < 0) return(E_INVALIDARG);
			mf.inputFlags |= WDM_MATCH_FORMAT;
		}

		if(e_camera->Attribute("frame_height") != NULL)
		{
			mf.biHeight = strtol(_bstr_t(e_camera->Attribute("frame_height")),NULL,10);
			if(mf.biHeight < 0) return(E_INVALIDARG);
			mf.inputFlags |= WDM_MATCH_FORMAT;
		}

		if(e_camera->Attribute("frame_rate") != NULL)
		{
			char* end_ptr = NULL;
			mf.frameRate = strtod(_bstr_t(e_camera->Attribute("frame_rate")),&end_ptr);
			if(mf.frameRate <= 0) return(E_INVALIDARG);
			mf.inputFlags |= WDM_MATCH_FORMAT;
		}

		e_pixel_format = doc_handle.FirstChild("dsvl_input").FirstChild("camera").FirstChild("pixel_format").FirstChild().Element();
		if(!e_pixel_format) return(E_INVALIDARG);
		char px_temp[MAX_PATH];
		sprintf(px_temp,"PIXELFORMAT_%s",e_pixel_format->Value());
		mf.pixel_format = StringToPX(px_temp);

		if(mf.pixel_format == PIXELFORMAT_RGB32)
		{
			if((e_pixel_format->Attribute("flip_h") != NULL) &&
				(_strnicmp("true",_bstr_t(e_pixel_format->Attribute("flip_h")),strlen("true")) == 0))
					media_format.flipH = true;
			if((e_pixel_format->Attribute("flip_v") != NULL) &&
				(_strnicmp("true",_bstr_t(e_pixel_format->Attribute("flip_v")),strlen("true")) == 0))
					media_format.flipV = true;
		}
	}
	else if(e_avi)
	{
		mf.inputDevice = ASYNC_FILE_INPUT_FILTER;
		swprintf(mf.sourceFilterName, sizeof(wchar_t) * MAX_PATH,_bstr_t(e_avi->Attribute("file_name")));

		if((e_avi->Attribute("use_reference_clock") != NULL) &&
		  (_strnicmp("false",_bstr_t(e_avi->Attribute("use_reference_clock")),strlen("false")) == 0))
				mf.inputFlags |= ASYNC_INPUT_DO_NOT_USE_CLOCK;

		if((e_avi->Attribute("loop_avi") != NULL) &&
		   (_strnicmp("true",_bstr_t(e_avi->Attribute("loop_avi")),strlen("true")) == 0))
			mf.inputFlags |= ASYNC_LOOP_VIDEO;

		if((e_avi->Attribute("render_secondary") != NULL) &&
		  (_strnicmp("true",_bstr_t(e_avi->Attribute("render_secondary")),strlen("true")) == 0))
			mf.inputFlags |= ASYNC_RENDER_SECONDARY_STREAMS;

		e_pixel_format = doc_handle.FirstChild("dsvl_input").FirstChild("avi_file").FirstChild("pixel_format").FirstChild().Element();
		if(!e_pixel_format) return(E_INVALIDARG);
		char px_temp[MAX_PATH];
		sprintf(px_temp,"PIXELFORMAT_%s",e_pixel_format->Value());
		mf.pixel_format = StringToPX(px_temp);

		if(mf.pixel_format == PIXELFORMAT_RGB32)
		{
			if((e_pixel_format->Attribute("flip_h") != NULL) &&
				(_strnicmp("true",_bstr_t(e_pixel_format->Attribute("flip_h")),strlen("true")) == 0))
				media_format.flipH = true;
			if((e_pixel_format->Attribute("flip_v") != NULL) &&
				(_strnicmp("true",_bstr_t(e_pixel_format->Attribute("flip_v")),strlen("true")) == 0))
				media_format.flipV = true;
		}
	}
	else return(E_INVALIDARG);


	// check for mf.defaultInputFlags and set default flags if necessary
	if(mf.defaultInputFlags)
	{
		switch(mf.inputDevice)
		{
		case(WDM_VIDEO_CAPTURE_FILTER): mf.inputFlags = default_VIDEO_INPUT_FLAGS;
			break;
		case(ASYNC_FILE_INPUT_FILTER):  mf.inputFlags = default_ASYNC_INPUT_FLAGS;
			break;
		default:                        return(E_INVALIDARG);
			break;
		};
	}


	HRESULT hr;

	CComPtr <IBaseFilter> pVideoSource = NULL;
	CComPtr <IBaseFilter> pStreamSplitter = NULL; // used in conjunction with the Async File Source filter
	CComPtr <IBaseFilter> pVideoDecoder = NULL;	// used for changing DV resolution
	CComPtr <IBaseFilter> pVideoRenderer = NULL;
	CComPtr <IAMStreamConfig> pStreamConfig = NULL;

	// OT-FIX 11/22/04 [thp]
	CComPtr <IBaseFilter> pSampleGrabber = NULL;

	FILTER_INFO filter_info;

	hr = CoCreateInstance (CLSID_FilterGraph, NULL, CLSCTX_INPROC,
		IID_IGraphBuilder, (void **) &graphBuilder);
	if (FAILED(hr)) return hr;

#ifdef _DEBUG
	hr = AddToRot(graphBuilder, &dwRegisterROT);
#endif

	// Create the capture graph builder
	hr = CoCreateInstance (CLSID_CaptureGraphBuilder2 , NULL, CLSCTX_INPROC,
		IID_ICaptureGraphBuilder2, (void **) &captureGraphBuilder);
	if (FAILED(hr))
		return hr;


	// OT-FIX 11/22/04 [thp]
	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&(pVideoRenderer));
	if (FAILED(hr))
		return hr;
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&(pSampleGrabber));
	if (FAILED(hr))
		return hr;
	if(FAILED(pSampleGrabber->QueryInterface(IID_ISampleGrabber,(void**)&sampleGrabber))) return(hr);

	AM_MEDIA_TYPE _mt;
	ZeroMemory(&_mt,sizeof(AM_MEDIA_TYPE));
	_mt.majortype = MEDIATYPE_Video;
	_mt.formattype = GUID_NULL;
	_mt.subtype = PXtoMEDIASUBTYPE(mf.pixel_format);
	hr = sampleGrabber->SetMediaType(&_mt);
	if (FAILED(hr))
		return hr;

	// Obtain interfaces for media control
	hr = graphBuilder->QueryInterface(IID_IMediaControl,(LPVOID *) &mediaControl);
	if (FAILED(hr))
		return hr;

	hr = graphBuilder->QueryInterface(IID_IMediaEvent, (LPVOID *) &mediaEvent);
	if (FAILED(hr))
		return hr;

	hr = captureGraphBuilder->SetFiltergraph(graphBuilder);
	if (FAILED(hr))
		return hr;

	// ###########################################################################################
	if (mf.inputDevice == WDM_VIDEO_CAPTURE_FILTER) {
		// ###########################################################################################

		hr = FindCaptureDevice(&pVideoSource,
			(mf.inputFlags & WDM_MATCH_FILTER_NAME ? mf.sourceFilterName : NULL),
			(mf.isDeviceName),
			(mf.inputFlags & WDM_MATCH_IEEE1394_ID ? mf.ieee1394_id : 0));
		if (FAILED(hr))
		{
			// Don't display a message because FindCaptureDevice will handle it
			return hr;
		}

		hr = graphBuilder->AddFilter(pVideoSource, L"Video Source");

		if (FAILED(hr))
		{
			AMErrorMessage(hr,"Couldn't add capture filter to graph!");
			return hr;
		}

		sourceFilter = pVideoSource;
		//capturePin = getPin(pVideoSource,PINDIR_OUTPUT,1); 
		if(FAILED(hr = getPin(pVideoSource,PINDIR_OUTPUT,1,capturePin))) return(hr); 

		// -------------
		hr = captureGraphBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,NULL,pVideoSource,IID_IAMStreamConfig,(void**)&pStreamConfig);
		if (FAILED(hr))
		{
			AMErrorMessage(hr,"Couldn't find IAMStreamConfig interface.");
			return hr;
		}

		// ---------------------------------------------------------------------------------
		// Unfortunately, WDM DV Video Capture Devices (such as consumer miniDV camcorders)
		// require special handling. Since the WDM source will only offer DVRESOLUTION_FULL,
		// any changes must be made through IIPDVDec on the DV Decoder filter.
		bool pinSupportsDV = CanDeliverDV(capturePin);

		// ---------------------------------------------------------------------------------
		if(pinSupportsDV)
		{
			CComPtr<IIPDVDec> pDVDec;
			// insert a DV decoder (CLSID_DVVideoCodec) into our graph.
			if(FAILED(hr = CoCreateInstance(CLSID_DVVideoCodec, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&(pVideoDecoder)))) return(hr);
			decoderFilter = pVideoDecoder;

			hr = graphBuilder->AddFilter(pVideoDecoder, L"Video Decoder");

			if(mf.inputFlags & WDM_MATCH_FORMAT)
				// since we're dealing with DV, there's only a limited range of possible resolutions:
				//
				//	 DVDECODERRESOLUTION_720x480 (PAL: 720x576)  = DVRESOLUTION_FULL 
				//	 DVDECODERRESOLUTION_360x240 (PAL: 360x288)  = DVRESOLUTION_HALF
				//	 DVDECODERRESOLUTION_180x120 (PAL: 180x144)	 = DVRESOLUTION_QUARTER
				//	 DVDECODERRESOLUTION_88x60   (PAL: 88x72)	 = DVRESOLUTION_DC

			{
				if(FAILED(pVideoDecoder->QueryInterface(IID_IIPDVDec,(void**)&pDVDec))) return(hr);

				int dvRes;
				if(FAILED(hr = pDVDec->get_IPDisplay(&dvRes))) return(hr); // get default resolution

				if((mf.biWidth == 720)	    && ((mf.biHeight == 480) || (mf.biHeight == 576))) dvRes = DVRESOLUTION_FULL;
				else if((mf.biWidth == 360) && ((mf.biHeight == 240) || (mf.biHeight == 288))) dvRes = DVRESOLUTION_HALF;
				else if((mf.biWidth == 180) && ((mf.biHeight == 120) || (mf.biHeight == 144))) dvRes = DVRESOLUTION_QUARTER;
				else if((mf.biWidth == 88)  && ((mf.biHeight == 60)  || (mf.biHeight == 72)))  dvRes = DVRESOLUTION_DC;

				if(FAILED(hr = pDVDec->put_IPDisplay(dvRes))) return(hr);
			}

			if((mf.inputFlags & WDM_SHOW_FORMAT_DIALOG)) // displaying the DV decoder's FILTER property page amounts to
				// the same as showing the WDM capture PIN property page.
			{
				if(FAILED(hr = DisplayFilterProperties(pVideoDecoder))) 
				{
					AMErrorMessage(hr,"Can't display filter properties.");
					//  non-critical error, no need to abort
				}
			}

		} // pinSupportsDV


		// ---------------------------------------------------------------------------------
		else // !pinSupportsDV
		{
			if(mf.inputFlags & WDM_MATCH_FORMAT)
			{
				AM_MEDIA_TYPE mt;
				if(FAILED(hr = MatchMediaTypes(capturePin, &mf, &mt)))
				{
					// automated media type selection failed -- display property page!
					mf.inputFlags &= WDM_SHOW_FORMAT_DIALOG;
				}
				else pStreamConfig->SetFormat(&mt);
			}

			if((mf.inputFlags & WDM_SHOW_FORMAT_DIALOG) && !pinSupportsDV)
				if(FAILED(hr = DisplayPinProperties(capturePin))) return(hr);

		} // !pinSupportsDV

		// ---------------------------------------------------------------------------------

		if(mf.inputFlags & WDM_SHOW_CONTROL_DIALOG)
			if(FAILED(hr = DisplayFilterProperties(sourceFilter))) return(hr);
		// ---------------------------------------------------------------------------------


		if(FAILED(pVideoSource->QueryInterface(IID_IAMCameraControl,(void**)&cameraControl)))
			cameraControl = NULL; // will be NULL anyway (replace with something intelligent)

		if(FAILED(pVideoSource->QueryInterface(IID_IAMDroppedFrames,(void**)&droppedFrames)))
			droppedFrames = NULL; // will be NULL anyway (replace with something intelligent)

		if(FAILED(pVideoSource->QueryInterface(IID_IAMVideoControl,(void**)&videoControl)))
			videoControl = NULL; // will be NULL anyway (replace with something intelligent)

		if(FAILED(pVideoSource->QueryInterface(IID_IAMVideoProcAmp,(void**)&videoProcAmp)))
			videoProcAmp = NULL; // will be NULL anyway (replace with something intelligent)


		// ###########################################################################################
	} else if (mf.inputDevice == ASYNC_FILE_INPUT_FILTER) {
		// ###########################################################################################

		if(FAILED(hr = CoCreateInstance(CLSID_AsyncReader, NULL, CLSCTX_INPROC_SERVER, 
			IID_IBaseFilter, (void**)&(pVideoSource)))) return(hr);

		CComPtr<IFileSourceFilter> pFileSource = NULL;		
		if(FAILED(pVideoSource->QueryInterface(IID_IFileSourceFilter,(void**)&pFileSource))) return(hr);
		if(FAILED(hr = pFileSource->Load(mf.sourceFilterName,NULL)))
		{
			char path[MAX_PATH];
			char* path_offset = NULL;
			if(SearchPath(NULL,_bstr_t(mf.sourceFilterName),NULL,MAX_PATH,path,&path_offset) > 0)
			{
				if(FAILED(hr = pFileSource->Load(_bstr_t(path),NULL)))
				{
					// file not found
					AMErrorMessage(hr,"Input file not found.");
					return(hr);
				}
			}
		}

		if(FAILED(hr = graphBuilder->AddFilter(pVideoSource, L"File Reader")))
		{
			AMErrorMessage(hr,"Couldn't add async file source filter to graph!");
			return hr;
		}

		if(FAILED(hr = CoCreateInstance(CLSID_AviSplitter, NULL, CLSCTX_INPROC_SERVER, 
			IID_IBaseFilter, (void**)&(pStreamSplitter)))) return(hr);

		if(FAILED(hr = graphBuilder->AddFilter(pStreamSplitter, L"Stream Splitter"))) return(hr);
		if(FAILED(hr = ConnectFilters(pVideoSource,1,pStreamSplitter,1)))
		{
			AMErrorMessage(hr,"Couldn't connect Async File Source to AVI Splitter!");
			return hr;
		}

		CComPtr<IPin> pStreamPin00 = NULL;
		if(FAILED(hr= getPin(pStreamSplitter,PINDIR_OUTPUT,1,pStreamPin00))) return(hr);
		//CComPtr<IPin> pStreamPin00 = getPin(pStreamSplitter,PINDIR_OUTPUT,1);
		if(pStreamPin00 == NULL || !CanDeliverVideo(pStreamPin00))
		{
			AMErrorMessage(hr,"AVI file format error. Substream 0x00 does not deliver MEDIATYPE_Video.");
			return(E_FAIL);
		}

		// -------------------------------------------------------------------------------------------
	} else return(E_INVALIDARG);
	// ###########################################################################################


	// OT-FIX 11/22/04 [thp]
	hr = graphBuilder->AddFilter(pSampleGrabber, L"Sample Grabber");

	// -------------
	hr = graphBuilder->AddFilter(pVideoRenderer, L"Video Renderer");
	// -------------
	// Render the capture pin on the video capture filter
	// Use this instead of g_pGraph->RenderFile


	// -------------------------------------------------------------------------------------------
	if (mf.inputDevice == WDM_VIDEO_CAPTURE_FILTER) {

		// OT-FIX 11/22/04 [thp]
		hr = AutoConnectFilters(pVideoSource,1,pSampleGrabber,1,graphBuilder);
		if(FAILED(hr)) return(hr);
		hr = AutoConnectFilters(pSampleGrabber,1,pVideoRenderer,1,graphBuilder);
		if(FAILED(hr)) return(hr);

		graphBuilder->SetDefaultSyncSource();

		// -------------------------------------------------------------------------------------------
	} else if (mf.inputDevice == ASYNC_FILE_INPUT_FILTER) {

		//hr = AutoConnectFilters(pStreamSplitter,1,pVideoRenderer,1,graphBuilder);

		// OT-FIX 11/22/04 [thp]
		hr = AutoConnectFilters(pStreamSplitter,1,pSampleGrabber,1,graphBuilder);

		if (FAILED(hr))
		{
			AMErrorMessage(hr,"Couldn't find a matching decoder filter for stream 0x00.\n"
				"Check if the required AVI codec is installed.");
			return hr;
		}
		// OT-FIX 11/22/04 [thp]
		hr = AutoConnectFilters(pSampleGrabber,1,pVideoRenderer,1,graphBuilder);
		if (FAILED(hr)) return(hr);

		if(mf.inputFlags & ASYNC_INPUT_DO_NOT_USE_CLOCK)
		{
			pVideoSource->SetSyncSource(NULL);
			pStreamSplitter->SetSyncSource(NULL);
			pVideoRenderer->SetSyncSource(NULL);

			// OT-FIX 11/22/04 [thp]
			pSampleGrabber->SetSyncSource(NULL);
		}

		if(mf.inputFlags & ASYNC_RENDER_SECONDARY_STREAMS)
		{
			IPin *pPin;
			CComPtr<IEnumPins> EnumPins;
			ULONG		fetched;
			PIN_INFO	pinfo;

			pStreamSplitter->EnumPins(&EnumPins);
			EnumPins->Reset();
			EnumPins->Next(1, &pPin, &fetched);
			pPin->QueryPinInfo(&pinfo);

			if(fetched > 0) do
			{
				if(pinfo.dir == PINDIR_OUTPUT)
				{
					IPin* pConnectedPin = NULL;
					if(pPin->ConnectedTo(&pConnectedPin) == VFW_E_NOT_CONNECTED)
						hr = graphBuilder->Render(pPin);
					if(pConnectedPin != NULL) pConnectedPin->Release();
				}
				pPin->Release();
				EnumPins->Next(1, &pPin, &fetched);
				pPin->QueryPinInfo(&pinfo);

			} while(fetched > 0);

		}

		// -------------------------------------------------------------------------------------------
	} else return(E_INVALIDARG);



	AM_MEDIA_TYPE mediaType;
	//CComPtr<IPin> rendererPin = getPin(pVideoRenderer,PINDIR_INPUT,1); 
	CComPtr<IPin> rendererPin = NULL;
	if(FAILED(hr = getPin(pVideoRenderer,PINDIR_INPUT,1,rendererPin))) return(hr);

	rendererPin->ConnectionMediaType(&mediaType);
	VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) mediaType.pbFormat;

	pVideoSource->QueryFilterInfo(&filter_info);
	// ISSUE: filter_info.achName will just contain an integer (e.g. '0001', probably in order of creation)
	// instead of the filter's "friendly name"
	media_format.sourceFilterName = (LPWSTR) CoTaskMemAlloc(sizeof(WCHAR)*(wcslen(filter_info.achName)+1));
	wcscpy(media_format.sourceFilterName, filter_info.achName);
	if(filter_info.pGraph != NULL) filter_info.pGraph->Release(); 

	media_format.subtype  = mediaType.subtype;
	media_format.biWidth  = pvi->bmiHeader.biWidth;
	media_format.biHeight = pvi->bmiHeader.biHeight;
	media_format.frameRate = avg2fps(pvi->AvgTimePerFrame);
	media_format.pixel_format = MEDIASUBTYPEtoPX(mediaType.subtype);
	FreeMediaType(mediaType);
	// -------------

	long _num_alloc = (MIN_ALLOCATOR_BUFFERS_PER_CLIENT * DEF_CONCURRENT_CLIENTS);
	CComPtr<IPin> sgPin = NULL;
	if(FAILED(hr=getPin(pSampleGrabber,PINDIR_INPUT,1,sgPin))) return(hr); 
	CComPtr<IMemAllocator> pAllocator = NULL;

	CComPtr<IMemInputPin> sgmiPin = NULL; 
	hr = sgPin->QueryInterface(IID_IMemInputPin, (void**)&sgmiPin);
	if (FAILED(hr)) return(hr);

	if(FAILED(hr = sgmiPin->GetAllocator(&pAllocator))) return(hr);
	if(FAILED(hr = pAllocator->Decommit())) return(hr);
	ALLOCATOR_PROPERTIES requestedProperties;
	ALLOCATOR_PROPERTIES actualProperties;
	pAllocator->GetProperties(&requestedProperties);
	if(requestedProperties.cBuffers != _num_alloc) requestedProperties.cBuffers = _num_alloc;
	hr = pAllocator->SetProperties(&requestedProperties,&actualProperties);

	// -------------
	m_bGraphIsInitialized = true;

	grabberFilter = pSampleGrabber;
	return(S_OK);
}


bool DSVL_GraphManager::IsGraphInitialized()
{
	return(m_bGraphIsInitialized);
}
