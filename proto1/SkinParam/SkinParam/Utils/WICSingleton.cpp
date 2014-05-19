// -------------------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// http://go.microsoft.com/fwlink/?LinkId=248926
// http://go.microsoft.com/fwlink/?LinkId=248929
//--------------------------------------------------------------------------------------

// Defines the WICImagingFactory singleton object
#pragma warning( disable : 4005 ) // disable duplicate macro definition warnings for vs2012

#include "WICSingleton.h"

namespace Utils {

	static bool g_WIC2 = false;

	IWICImagingFactory* GetWIC() {
		static IWICImagingFactory* s_Factory = nullptr;

		if ( s_Factory )
			return s_Factory;

	#ifdef WIC_USE_FACTORY_PROXY
		HINSTANCE hInst = LoadLibraryEx( TEXT("WindowsCodecs.dll"), nullptr, 0 );
		if ( !hInst )
			return nullptr;

		typedef HRESULT (WINAPI *WICCREATEFACTORY)(UINT, IWICImagingFactory**);
		WICCREATEFACTORY pFactory = (WICCREATEFACTORY) GetProcAddress(hInst, "WICCreateImagingFactory_Proxy");
		if ( !pFactory )
		{
			FreeLibrary( hInst );
			return nullptr;
		}

	#if(_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/) || defined(_WIN7_PLATFORM_UPDATE)
		HRESULT hr = pFactory( WINCODEC_SDK_VERSION2, &s_Factory );
		if ( SUCCEEDED(hr) )
		{
			// WIC2 is available on Windows 8 and Windows 7 SP1 with KB 2670838 installed
			g_WIC2 = true;
		}
		else
		{
			hr = pFactory( WINCODEC_SDK_VERSION1, &s_Factory );
			if ( FAILED(hr) )
			{
				FreeLibrary( hInst );
				s_Factory = nullptr;
				return nullptr;
			}
		}
	#else
		HRESULT hr = pFactory( WINCODEC_SDK_VERSION, &s_Factory );
		if ( FAILED(hr) )
		{
			FreeLibrary( hInst );
			s_Factory = nullptr;
			return nullptr;
		}
	#endif

	#elif(_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/) || defined(_WIN7_PLATFORM_UPDATE)
		HRESULT hr = CoCreateInstance(
			CLSID_WICImagingFactory2,
			nullptr,
			CLSCTX_INPROC_SERVER,
			__uuidof(IWICImagingFactory2),
			(LPVOID*)&s_Factory
			);

		if ( SUCCEEDED(hr) )
		{
			// WIC2 is available on Windows 8 and Windows 7 SP1 with KB 2670838 installed
			g_WIC2 = true;
		}
		else
		{
			hr = CoCreateInstance(
				CLSID_WICImagingFactory1,
				nullptr,
				CLSCTX_INPROC_SERVER,
				__uuidof(IWICImagingFactory),
				(LPVOID*)&s_Factory
				);

			if ( FAILED(hr) )
			{
				s_Factory = nullptr;
				return nullptr;
			}
		}
	#else
		HRESULT hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			nullptr,
			CLSCTX_INPROC_SERVER,
			__uuidof(IWICImagingFactory),
			(LPVOID*)&s_Factory
			);

		if ( FAILED(hr) )
		{
			s_Factory = nullptr;
			return nullptr;
		}
	#endif

		return s_Factory;
	}

} // namespace Utils;
