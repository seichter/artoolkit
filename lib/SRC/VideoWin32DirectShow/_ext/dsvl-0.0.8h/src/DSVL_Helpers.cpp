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

#include <atlbase.h>
#include <dshow.h>
#include <qedit.h>
#include <comutil.h>

#include "DSVL_Helpers.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>


// -----------------------------------------------------------------------------------------------------------------
double avg2fps(REFERENCE_TIME AvgTimePerFrame, int precision)
{ 
	return(RoundDouble(10000000.0 / (double)AvgTimePerFrame,precision)); 
}
// -----------------------------------------------------------------------------------------------------------------
REFERENCE_TIME fps2avg(double fps)
{
	return((REFERENCE_TIME)(1.0/fps*10000000.0));
}
// -----------------------------------------------------------------------------------------------------------------


HRESULT WINAPI CopyMediaType( AM_MEDIA_TYPE *pmtTarget,
                              const AM_MEDIA_TYPE *pmtSource )
{
    *pmtTarget = *pmtSource;

    if( !pmtSource || !pmtTarget ) return S_FALSE;

    if( pmtSource->cbFormat && pmtSource->pbFormat )
    {
        pmtTarget->pbFormat = (PBYTE)CoTaskMemAlloc( pmtSource->cbFormat );
        if( pmtTarget->pbFormat == NULL )
        {
            pmtTarget->cbFormat = 0;
            return E_OUTOFMEMORY;
        }
        else
        {
            CopyMemory( (PVOID)pmtTarget->pbFormat, (PVOID)pmtSource->pbFormat,
                        pmtTarget->cbFormat );
        }
    }
    if( pmtTarget->pUnk != NULL )
    {
        pmtTarget->pUnk->AddRef();
    }

    return S_OK;
}



void FreeMediaType(AM_MEDIA_TYPE& mt)
{
	if (mt.cbFormat != 0)
	{
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if (mt.pUnk != NULL)
	{
		// Unecessary because pUnk should not be used, but safest.
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}
}

void DeleteMediaType(AM_MEDIA_TYPE *pmt)
{
	if (pmt != NULL)
	{
		FreeMediaType(*pmt); // See FreeMediaType for the implementation.
		CoTaskMemFree(pmt);
	}
}



HRESULT getPin(IBaseFilter *flt, PIN_DIRECTION dir, int number, CComPtr<IPin> &pRetPin)
{
	int n=0;
	IPin* Pin;
	IEnumPins*	EnumPins;
	ULONG		fetched;
	PIN_INFO	pinfo;

	flt->EnumPins(&EnumPins);
	EnumPins->Reset();
	EnumPins->Next(1, &Pin, &fetched);
	Pin->QueryPinInfo(&pinfo);
	//pinfo.pFilter->Release();

	do
	{
		// the pFilter member has an outstanding ref count -> release it, we do not use it anyways!
		pinfo.pFilter->Release();
		pinfo.pFilter = 0;
		if(pinfo.dir == dir)
		{
			n++;
			if(n==number) 
			{
				EnumPins->Release();
				pRetPin = Pin;
				return S_OK;
			}
			else
			{
				Pin->Release();
				EnumPins->Next(1, &Pin, &fetched);
				if(fetched == 0) // no more pins
				{
					EnumPins->Release();
					pRetPin = NULL;
					return(E_FAIL);
				}
				Pin->QueryPinInfo(&pinfo);
			}
		}
		else //if (pinfo.dir != dir)
		{
			Pin->Release();
			EnumPins->Next(1, &Pin, &fetched);
			if(fetched == 0) // no more pins
			{
				EnumPins->Release();
				pRetPin = NULL;
				return(E_FAIL);
			}
			Pin->QueryPinInfo(&pinfo);
			//pinfo.pFilter->Release();
		}
	} while(Pin != NULL);

	EnumPins->Release();
	return E_FAIL;
}
// -----------------------------------------------------------------------------------------------------------------
HRESULT ConnectFilters(IBaseFilter *filter_out, int out_pin_nr, 
					   IBaseFilter *in_filter, int in_pin_nr)
{
	HRESULT hr;
	CComPtr<IPin> OutPin = NULL;
	CComPtr<IPin> InPin = NULL;
	if(FAILED(hr = getPin(filter_out,PINDIR_OUTPUT,out_pin_nr,OutPin))) return(hr);
	if(FAILED(hr = getPin(in_filter,PINDIR_INPUT,in_pin_nr,InPin))) return(hr);
	if(OutPin == NULL || InPin== NULL) return(E_FAIL);
	
	if(FAILED(hr = OutPin->Connect(InPin,NULL))) return(E_FAIL);
	else return(S_OK);
}
// -----------------------------------------------------------------------------------------------------------------

HRESULT AutoConnectFilters(IBaseFilter *filter_out, int out_pin_nr, IBaseFilter *in_filter,
											  int in_pin_nr, IGraphBuilder *pGraphBuilder)
{
	HRESULT hr;
	CComPtr<IPin> OutPin  = NULL;
	CComPtr<IPin> InPin  = NULL;
	if(FAILED(hr = getPin(filter_out,PINDIR_OUTPUT,out_pin_nr,OutPin))) return(hr);
	if(FAILED(hr = getPin(in_filter,PINDIR_INPUT,in_pin_nr,InPin))) return(hr);
	if(OutPin == NULL || InPin== NULL) return(E_FAIL);
	
	if(FAILED(hr = pGraphBuilder->Connect(OutPin,InPin))) return(E_FAIL);
	else return(S_OK);
}
// -----------------------------------------------------------------------------------------------------------------

char* strtok_prefix(char* str, const char* prefix)
{
	if(strstr(str,prefix) != 0)
		return(str+strlen(prefix));
	else return(NULL);
}
// -----------------------------------------------------------------------------------------------------------------

void uuidToString(char* dest, int dest_size, GUID* uuid_p)
{
	ZeroMemory(dest,dest_size);
	WideCharToMultiByte(CP_ACP,WC_DEFAULTCHAR,CComBSTR(*uuid_p).m_str,
						CComBSTR(*uuid_p).Length(),dest,dest_size,NULL,NULL);
}

// -----------------------------------------------------------------------------------------------------------------

float Round(const float &number, const int num_digits)
{
	float doComplete5i, doComplete5(number * powf(10.0f, (float) (num_digits + 1)));

	if(number < 0.0f)
		doComplete5 -= 5.0f;
	else
		doComplete5 += 5.0f;

	doComplete5 /= 10.0f;
	modff(doComplete5, &doComplete5i);

	return doComplete5i / powf(10.0f, (float) num_digits);
}

// -----------------------------------------------------------------------------------------------------------------

double RoundDouble(double doValue, int nPrecision)
{
	static const double doBase = 10.0f;
	double doComplete5, doComplete5i;

	doComplete5 = doValue * pow(doBase, (double) (nPrecision + 1));

	if(doValue < 0.0f)
		doComplete5 -= 5.0f;
	else
		doComplete5 += 5.0f;

	doComplete5 /= doBase;
	modf(doComplete5, &doComplete5i);

	return doComplete5i / pow(doBase, (double) nPrecision);
}

// -----------------------------------------------------------------------------------------------------------------

HRESULT AddToRot(IUnknown *pUnkGraph, DWORD *pdwRegister) 
// see Microsoft DirectX SDK 9.0 "Loading a Graph From an External Process" 
{
	CComPtr<IMoniker> pMoniker;
	CComPtr<IRunningObjectTable> pROT;
	if (FAILED(GetRunningObjectTable(0, &pROT))) {
		return E_FAIL;
	}
	CHAR wsz[256];
	sprintf(wsz, "FilterGraph %08x pid %08x", (DWORD_PTR)pUnkGraph, GetCurrentProcessId());
	HRESULT hr = CreateItemMoniker(L"!", _bstr_t(wsz), &pMoniker);
	if (SUCCEEDED(hr)) {
		hr = pROT->Register(0, pUnkGraph, pMoniker, pdwRegister);
	}
	return hr;
}

// -----------------------------------------------------------------------------------------------------------------

void RemoveFromRot(DWORD pdwRegister)
{
	CComPtr<IRunningObjectTable> pROT;
	if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) {
		pROT->Revoke(pdwRegister);
	}
}

// -----------------------------------------------------------------------------------------------------------------

HRESULT DisplayPinProperties(CComPtr<IPin> pSrcPin, HWND hWnd)
{
	CComPtr<ISpecifyPropertyPages> pPages;

	HRESULT hr = pSrcPin->QueryInterface(IID_ISpecifyPropertyPages, (void**)&pPages);
	if (SUCCEEDED(hr))
	{
		PIN_INFO PinInfo;
		pSrcPin->QueryPinInfo(&PinInfo);

		CAUUID caGUID;
		pPages->GetPages(&caGUID);

		OleCreatePropertyFrame(
			hWnd,
			0,
			0,
			L"Property Sheet",
			1,
			(IUnknown **)&(pSrcPin.p),
			caGUID.cElems,
			caGUID.pElems,
			0,
			0,
			NULL);
		CoTaskMemFree(caGUID.pElems);
		PinInfo.pFilter->Release();
	}
	else return(hr);

	return(S_OK);
}

// -----------------------------------------------------------------------------------------------------------------

HRESULT DisplayFilterProperties(IBaseFilter *pFilter, HWND hWnd)
{
	CComPtr<ISpecifyPropertyPages> pProp;
	HRESULT hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pProp);
	if (SUCCEEDED(hr)) 
	{
		// Get the filter's name and IUnknown pointer.
		FILTER_INFO FilterInfo;
		pFilter->QueryFilterInfo(&FilterInfo); 

		// Show the page. 
		CAUUID caGUID;
		pProp->GetPages(&caGUID);
		OleCreatePropertyFrame(
			hWnd,                   // Parent window
			0, 0,                   // (Reserved)
			FilterInfo.achName,     // Caption for the dialog box
			1,                      // Number of objects (just the filter)
			(IUnknown **)&pFilter,  // Array of object pointers. 
			caGUID.cElems,          // Number of property pages
			caGUID.pElems,          // Array of property page CLSIDs
			0,                      // Locale identifier
			0, NULL);               // Reserved

		// Clean up.
		if(FilterInfo.pGraph != NULL) FilterInfo.pGraph->Release(); 
		CoTaskMemFree(caGUID.pElems);
	}
	return(hr);
}


// -----------------------------------------------------------------------------------------------------------------

HRESULT FindCaptureDevice(IBaseFilter ** ppSrcFilter, 
						  LPWSTR filterNameSubstring,
						  bool matchDeviceName,
						  char* ieee1394id_str)
{
	HRESULT hr;
	IBaseFilter * pSrc = NULL;
	CComPtr <IMoniker> pMoniker =NULL;
	ULONG cFetched;

	// Create the system device enumerator
	CComPtr <ICreateDevEnum> pDevEnum =NULL;

	hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
		IID_ICreateDevEnum, (void ** ) &pDevEnum);
	if (FAILED(hr))
	{
		AMErrorMessage(hr,"Couldn't create system enumerator!");
		return hr;
	}

	// Create an enumerator for the video capture devices
	CComPtr <IEnumMoniker> pClassEnum = NULL;

	hr = pDevEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
	if (FAILED(hr))
	{
		AMErrorMessage(hr,"Couldn't create system enumerator!");
		return hr;
	}

	// If there are no enumerators for the requested type, then 
	// CreateClassEnumerator will succeed, but pClassEnum will be NULL.
	if (pClassEnum == NULL)
	{
		MessageBox(NULL,TEXT("No video capture device was detected.\r\n\r\n")
			TEXT("You need at least one WDM video capture device, such as a USB WebCam.\r\n"),
			TEXT("No Video Capture Hardware"), MB_OK | MB_ICONINFORMATION);
		return E_FAIL;
	}

	while(S_OK == (pClassEnum->Next (1, &pMoniker, &cFetched)))
	{
		CComPtr<IPropertyBag> pProp;
		pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pProp);
		VARIANT varName;
		VariantInit(&varName); // Try to match the friendly name.
		HRESULT name_hr = pProp->Read(L"FriendlyName", &varName, 0); 

		//		(SUCCEEDED(hr) && (wcsstr(varName.bstrVal, filterNameSubstring) != NULL)))
		if(filterNameSubstring != NULL)
		{
			// Bind Moniker to a filter object
			hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)&pSrc);

			if(matchDeviceName)
			{
				LPOLESTR strName = NULL;
				hr = pMoniker->GetDisplayName(NULL, NULL, &strName);
				if(wcsstr(strName, filterNameSubstring) != NULL)
				{
					*ppSrcFilter = pSrc;
					return(S_OK);
				}
			}
			else // matchFriendlyName
			{
				if(SUCCEEDED(name_hr) && wcsstr(varName.bstrVal, filterNameSubstring) != NULL)
				{
					// check for matching IEEE-1394 identifier
					if(ieee1394id_str != NULL)
					{
						IAMExtDevice *pExtDev = NULL;
						hr = pSrc->QueryInterface(IID_IAMExtDevice, (void**)&pExtDev);
						if(SUCCEEDED(hr))
						{
							LPOLESTR ole_str = NULL;
							bool matching_id = false;
							hr = pExtDev->get_ExternalDeviceID(&ole_str);

							unsigned __int64 msdv_id = *((unsigned __int64*) ole_str);
							if(ole_str != NULL) CoTaskMemFree(ole_str);

							char* temp_str = (char*) CoTaskMemAlloc(sizeof(char) * MAX_PATH);
							_ui64toa(msdv_id,temp_str,16);
							if(strcmp(ieee1394id_str,temp_str) == 0) matching_id = true;
							CoTaskMemFree(temp_str);

							if(SUCCEEDED(hr) && matching_id)
							{
								*ppSrcFilter = pSrc;
								return(S_OK);
							}
							else // pExtDev->get_ExternalDeviceID() failed || identifier mismatch
							{
								pSrc->Release();
							}
						}
						else
						{
							pSrc->Release();
						}
					}
					else // (ieee1394id == 0)
					{
						*ppSrcFilter = pSrc;
						return(S_OK);
					}
				}
				else // friendlyName substrings don't match
				{
					pSrc->Release();
				}
			}
		}
		VariantClear(&varName);
		pMoniker = NULL; // Release for the next loop.
	}

	pClassEnum->Reset();

	// Use the first video capture device on the device list.
	// Note that if the Next() call succeeds but there are no monikers,
	// it will return S_FALSE (which is not a failure).  Therefore, we
	// check that the return code is S_OK instead of using SUCCEEDED() macro.
	if (S_OK == (pClassEnum->Next (1, &pMoniker, &cFetched)))
	{
		// Bind Moniker to a filter object
		hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)&pSrc);
		if (FAILED(hr))
		{
			AMErrorMessage(hr,"Couldn't bind moniker to filter object!");
			return hr;
		}
	}
	else
	{
		MessageBox(NULL,"Unable to access video capture device!","ERROR",MB_OK);
		return E_FAIL;
	}

	// Copy the found filter pointer to the output parameter.
	// Do NOT Release() the reference, since it will still be used
	// by the calling function.
	*ppSrcFilter = pSrc;

	return hr;
}

// -----------------------------------------------------------------------------------------------------------------

bool CanDeliverVideo(IPin *pin) 
// checks if a pin can deliver MEDIAFORMAT_Video
{
	HRESULT hr;
	CComPtr <IEnumMediaTypes> enum_mt;
	pin->EnumMediaTypes(&enum_mt);
	enum_mt->Reset();
	AM_MEDIA_TYPE *p_mt = NULL;
	while(S_OK == (hr = enum_mt->Next(1,&p_mt,NULL)))
	{
		if(p_mt->majortype == MEDIATYPE_Video)
		{
			DeleteMediaType(p_mt);
			return(true);
		}
		else DeleteMediaType(p_mt);
	}

	return(false);
}

// -----------------------------------------------------------------------------------------------------------------
bool CanDeliverDV(IPin *pin) 
// checks if a pin can deliver compressed DV formats
// (refer to Microsoft DirectX Documentation, "DV Video Subtypes" for more information)

/*
FOURCC	GUID				Data Rate	Description 
'dvsl'	MEDIASUBTYPE_dvsl	12.5 Mbps	SD-DVCR 525-60 or SD-DVCR 625-50 
'dvsd'	MEDIASUBTYPE_dvsd	25 Mbps		SDL-DVCR 525-60 or SDL-DVCR 625-50 
'dvhd'	MEDIASUBTYPE_dvhd	50 Mbps		HD-DVCR 1125-60 or HD-DVCR 1250-50 
*/
{
	HRESULT hr;
	CComPtr <IEnumMediaTypes> enum_mt;
	pin->EnumMediaTypes(&enum_mt);
	enum_mt->Reset();
	AM_MEDIA_TYPE *p_mt = NULL;
	while(S_OK == (hr = enum_mt->Next(1,&p_mt,NULL)))
	{
		if((p_mt->subtype == MEDIASUBTYPE_dvsl) ||
			(p_mt->subtype == MEDIASUBTYPE_dvsd) ||
			(p_mt->subtype == MEDIASUBTYPE_dvhd))
		{
			DeleteMediaType(p_mt);
			return(true);
		}
		else DeleteMediaType(p_mt);
	}

	return(false);
}

// -----------------------------------------------------------------------------------------------------------------
HRESULT AMErrorMessage(HRESULT hr, char* error_title)
{
	TCHAR buffer[MAX_PATH];
	DWORD cRet = AMGetErrorText(hr,buffer,MAX_PATH);
	if(cRet > 0) MessageBox(NULL,_T(buffer),error_title,MB_OK);
	else return(E_FAIL);
	return(S_OK);
}

// -----------------------------------------------------------------------------------------------------------------

void FlipImageRGB32(BYTE* pBuf, int width, int height, 
					bool flipImageH, bool flipImageV)
{
	DWORD *ptr = (DWORD*)pBuf;
	int pixelCount = width * height;

	if (!pBuf)
		return;

	// Added code for more image manipulations
	// Gerhard Reitmayr <reitmayr@ims.tuwien.ac.at

	if( flipImageV )
	{
		if( flipImageH )
		{
			// both flips set -> rotation about 180 degree
			for (int index = 0; index < pixelCount/2; index++)
			{
				ptr[index] = ptr[index] ^ ptr[pixelCount - index - 1];
				ptr[pixelCount - index - 1] = ptr[index] ^ ptr[pixelCount - index - 1];
				ptr[index] = ptr[index] ^ ptr[pixelCount - index - 1];
			}
		} 
		else  
		{
			// only vertical flip 
			for( int line = 0; line < height/2; line++ )
				for( int pixel = 0; pixel < width; pixel ++ )
				{
					ptr[line*width+pixel] = ptr[line*width+pixel] ^ ptr[pixelCount - line*width - (width - pixel )];
					ptr[pixelCount - line*width - (width - pixel )] = ptr[line*width+pixel] ^ ptr[pixelCount - line*width - (width - pixel )];
					ptr[line*width+pixel] = ptr[line*width+pixel] ^ ptr[pixelCount - line*width - (width - pixel )];
				}
		}
	}
	else 
	{
		if( flipImageH )
		{
			// only horizontal flip
			for( int line = 0; line < height; line++ )
				for( int pixel = 0; pixel < width/2; pixel ++ )
				{
					ptr[line*width+pixel] = ptr[line*width+pixel] ^ ptr[line*width + (width - pixel )];
					ptr[line*width + (width - pixel )] = ptr[line*width+pixel] ^ ptr[line*width + (width - pixel )];
					ptr[line*width+pixel] = ptr[line*width+pixel] ^ ptr[line*width + (width - pixel )];
				}
		}
	}
}

CAutoLock::CAutoLock( CCritSec *plook ) : m_pLock(plook)
{
	plook->Enter();
}

CAutoLock::~CAutoLock()
{
	if(m_pLock) m_pLock->Leave();
}

