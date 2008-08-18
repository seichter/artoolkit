 /* ========================================================================
  * PROJECT: DirectShow Video Processing Library
  * File:	 IEEE 1394 DeviceID Tool
  * Version: 0.0.2 (02/12/2003)
  * ========================================================================
  * Author:  Thomas Pintaric, Vienna University of Technology
  * Contact: pintaric@ims.tuwien.ac.at http://ims.tuwien.ac.at/~thomas
  * =======================================================================
  *
  * Copyright (C) 2003  Vienna University of Technology
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Lesser General Public
  * License as published by the Free Software Foundation; either
  * version 2.1 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Lesser General Public License for more details.
  *
  * You should have received a copy of the GNU Lesser General Public
  * License along with this library; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  *
  * For further information please contact Thomas Pintaric under
  * <pintaric@ims.tuwien.ac.at> or write to Thomas Pintaric,
  * Vienna University of Technology, Favoritenstr. 9-11/E188/2, A-1040 Vienna,
  * Austria.
  * ========================================================================*/

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
//#include <streams.h>
#include <atlbase.h>
#include <dshow.h>
#include <qedit.h>
#include <atlbase.h>
#include <comutil.h>
#include <stdio.h>
#include <tchar.h>

HRESULT Enumerate();

int _tmain(int argc, _TCHAR* argv[])
{
	printf("========================================================================\n");
	printf("TITLE:   IEEE 1394 DeviceID Tool, v0.0.2\n");
	printf("Author:  Thomas Pintaric, Vienna University of Technology\n");
	printf("Contact: thomas@ims.tuwien.ac.at, http://ims.tuwien.ac.at/~thomas\n");
	printf("========================================================================\n");
	printf("PLEASE NOTE: This program comes with ABSOLUTELY NO WARRANTY.\n");
	printf("This is free software, and you are welcome to redistribute it under\n");
	printf("certain conditions. See http://www.gnu.org/copyleft/gpl.html for details.\n\n");

	CoInitialize(NULL);
	HRESULT hr = Enumerate();
	CoUninitialize();
	return(hr);
}


HRESULT Enumerate()
{
    HRESULT hr;
	IBaseFilter  *pSrc = NULL;
    CComPtr <IMoniker> pMoniker =NULL;
    ULONG cFetched;
	char str_tmp[MAX_PATH];
   
	printf("Enumerating WDM device drivers...\n\n");

    // Create the system device enumerator
    CComPtr <ICreateDevEnum> pDevEnum =NULL;

    hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
        IID_ICreateDevEnum, (void ** ) &pDevEnum);
    if (FAILED(hr))
    {
		printf("Couldn't create system enumerator!\n");
        return(hr);
    }

    // Create an enumerator for the video capture devices
    CComPtr <IEnumMoniker> pClassEnum = NULL;

    hr = pDevEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
    if (FAILED(hr))
    {
		printf("Couldn't create system enumerator!\n");
        return(hr);
    }

    // If there are no enumerators for the requested type, then 
    // CreateClassEnumerator will succeed, but pClassEnum will be NULL.
    if (pClassEnum == NULL)
    {
		printf("No video capture hardware detected!\n");
        return(E_FAIL);
    }

	while(S_OK == (pClassEnum->Next (1, &pMoniker, &cFetched)))
	{
		CComPtr<IPropertyBag> pProp;
        pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pProp);
        VARIANT varName;
        VariantInit(&varName);
        hr = pProp->Read(L"FriendlyName", &varName, 0); 
		if(SUCCEEDED(hr))
		{
			strcpy(str_tmp,_bstr_t(varName.bstrVal));
			printf("--Device name: %s\n",str_tmp);
	        // Bind Moniker to a filter object
		    hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)&pSrc);
			if(FAILED(hr)) 
			{
				printf("  Error: could not bind to filter object.\n");
				goto next;
			}

			// -----------------------------------------------------------------------
			IAMExtDevice *pExtDev = NULL;
			hr = pSrc->QueryInterface(IID_IAMExtDevice, (void**)&pExtDev);
			if(SUCCEEDED(hr)) 
			{
				printf("  IAMExtDevice properties:\n");

				long l;
				hr = pExtDev->get_DevicePort(&l);
				if(SUCCEEDED(hr))
				{
					printf("    DevicePort: %i",l);
					switch(l)
					{
					  case(DEV_PORT_1394): printf("IEEE 1394 Bus"); 
					                       break;
					  default :            printf("(%i) non IEEE 1394\n",l);
										   pExtDev->Release();
										   goto next;
						                   break;
					};
					printf("\n");
				}

				LPOLESTR ole_str = NULL;
				hr = pExtDev->get_ExternalDeviceID(&ole_str);
				if(SUCCEEDED(hr))
				{
					unsigned __int64 msdv_id = *((unsigned __int64*) ole_str);
					printf("    ExternalDeviceID: %s (hexadecimal)\n",_ui64toa(msdv_id,str_tmp,16));
					printf("      >> Unique 64-bit identifier, as defined by IEEE 1394.\n");

					CoTaskMemFree(ole_str);
				}

				hr = pExtDev->get_ExternalDeviceVersion(&ole_str);
				if(SUCCEEDED(hr))
				{
					strcpy(str_tmp,_bstr_t(ole_str));
					printf("    ExternalDeviceVersion: %s\n",str_tmp);
					CoTaskMemFree(ole_str);
				}

				hr = pExtDev->get_DevicePower(&l);
				if(SUCCEEDED(hr))
				{
					printf("    DevicePower: %i",l);
					switch(l)
					{
					  case(ED_POWER_OFF):     printf("off"); break;
					  case(ED_POWER_ON):      printf("on"); break;
					  case(ED_POWER_STANDBY): printf("standby"); break;
					  default :               printf("unknown"); break;
					};
					printf("\n");
				}

				pExtDev->Release();
			}
			else
			{
				printf("  IAMExtDevice not supported.\n");
			}
	
			pSrc->Release();
		}
		printf("\n");
		next: // LABEL next:

        VariantClear(&varName);
        pMoniker = NULL; // Release for the next loop.
    }

	pClassEnum->Reset();

    return hr;
}
