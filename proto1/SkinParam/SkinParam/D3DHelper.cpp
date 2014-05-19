
/*
    Copyright(c) 2013-2014 Yifan Wu.

    This file is part of SkinParam.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

/**
* D3D helper functions
*/

#include "stdafx.h"

#include "D3DHelper.h"
#include "DirectXTex\WICTextureLoader\WICTextureLoader.h"
#include "DirectXTex\DDSTextureLoader\DDSTextureLoader.h"
#include "WICSingleton.h"
#include <cmath>
#include <cstring>
#include <xhash>
#include <string>
#include <unordered_map>

using namespace Skin;
using namespace Utils;
using namespace DirectX;

void D3DHelper::checkFailure(HRESULT hr, const TString& prompt) {
	if (FAILED(hr)) {
		TStringStream ss;
		ss << (prompt) << _T("\nError description: ") << DXGetErrorDescription(hr);
		::MessageBox(AfxGetMainWnd()->m_hWnd, ss.str().c_str(), _T("Fatal Error"), MB_OK | MB_ICONERROR);
		::exit(EXIT_FAILURE);
	}
}

namespace Skin { namespace D3DHelper {
	using namespace std;
	// Implements shader caching
	struct ShaderCacheKey {
		TString fileName;
		string profile;
		string entryPoint;

		static hash<TString> tstring_hash;
		static hash<string> string_hash;

		ShaderCacheKey(TString fileName, string profile, string entryPoint) {
			this->fileName = fileName;
			this->profile = profile;
			this->entryPoint = entryPoint;
		}
		friend static bool operator==(const ShaderCacheKey& one, const ShaderCacheKey& two) {
			return one.fileName == two.fileName && one.profile == two.profile && one.entryPoint == two.entryPoint;
		}
		size_t hash() const {
			return tstring_hash(fileName) ^ string_hash(profile) ^ string_hash(entryPoint);
		}
	};
	hash<TString> ShaderCacheKey::tstring_hash;
	hash<string> ShaderCacheKey::string_hash;

	class ShaderCacheKeyHasher : public unary_function<const ShaderCacheKey&, size_t> {
	public:
		size_t operator()(const ShaderCacheKey& key) {
			return key.hash();
		}
	};
	class ShaderCacheKeyEqualTo : public binary_function<const ShaderCacheKey&, const ShaderCacheKey&, bool> {
	public:
		size_t operator()(const ShaderCacheKey& one, const ShaderCacheKey& two) {
			return one == two;
		}
	};

	struct VertexShaderCacheValue {
		CComPtr<ID3D11VertexShader> pVertexShader;
		CComPtr<ID3DBlob> pBlob;
		VertexShaderCacheValue() {}
		VertexShaderCacheValue(ID3D11VertexShader* pShader, ID3DBlob* pBlob)
			: pVertexShader(pShader), pBlob(pBlob) { }
	};
	typedef CComPtr<ID3D11HullShader> HullShaderCacheValue;
	typedef CComPtr<ID3D11DomainShader> DomainShaderCacheValue;
	typedef CComPtr<ID3D11GeometryShader> GeometryShaderCacheValue;
	typedef CComPtr<ID3D11PixelShader> PixelShaderCacheValue;

	static unordered_map<ShaderCacheKey, VertexShaderCacheValue, ShaderCacheKeyHasher, ShaderCacheKeyEqualTo> g_vscache;
	static unordered_map<ShaderCacheKey, HullShaderCacheValue, ShaderCacheKeyHasher, ShaderCacheKeyEqualTo> g_hscache;
	static unordered_map<ShaderCacheKey, DomainShaderCacheValue, ShaderCacheKeyHasher, ShaderCacheKeyEqualTo> g_dscache;
	static unordered_map<ShaderCacheKey, GeometryShaderCacheValue, ShaderCacheKeyHasher, ShaderCacheKeyEqualTo> g_gscache;
	static unordered_map<ShaderCacheKey, PixelShaderCacheValue, ShaderCacheKeyHasher, ShaderCacheKeyEqualTo> g_pscache;
} } // namespace Skin::D3DHelper

void D3DHelper::clearShaderCache() {
	g_vscache.clear();
	g_hscache.clear();
	g_dscache.clear();
	g_gscache.clear();
	g_pscache.clear();
}

HRESULT D3DHelper::compileShader(const TString& strFileName, const char* szEntryPoint, const char* szShaderModel,
								 ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PREFER_FLOW_CONTROL;
#else
	// Might be a bug in d3dcompiler_43.dll, see https://gist.github.com/aras-p/3236705
	// Hull shader optimization is bugged
	if (_strnicmp(szShaderModel, "hs_", 3) == 0)
		dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	CComPtr<ID3DBlob> pErrorBlob;
	hr = D3DX11CompileFromFile(strFileName.c_str(), NULL, NULL, szEntryPoint, szShaderModel, 
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	if (FAILED(hr)) {
		if( pErrorBlob != NULL ) {
			TRACE("[D3DHelper::compileShader] %s\t", (char*)pErrorBlob->GetBufferPointer());
			MessageBoxA(AfxGetMainWnd()->m_hWnd, (char*)pErrorBlob->GetBufferPointer(), "Error Compiling Shader", MB_OK | MB_ICONERROR);
		}
		return hr;
	}

	return S_OK;
}

HRESULT D3DHelper::loadVertexShaderAndLayout(ID3D11Device* pDevice,
											 const TString& strFileName, const char* szEntryPoint,
											 D3D11_INPUT_ELEMENT_DESC aLayoutDesc[], UINT numLayoutDesc,
											 ID3D11VertexShader** ppVertexShader, ID3D11InputLayout** ppInputLayout)
{
	HRESULT hr;

	CComPtr<ID3DBlob> pVSBlob = nullptr;

	const char* PROFILE = "vs_5_0";
	ShaderCacheKey key(strFileName, PROFILE, szEntryPoint);
	auto iter = g_vscache.find(key);
	if (iter != g_vscache.end()) {
		*ppVertexShader = iter->second.pVertexShader;
		(*ppVertexShader)->AddRef();
		pVSBlob = iter->second.pBlob;
	} else {
		// Compile the vertex shader
		hr = compileShader(_T("Shaders/") + strFileName, szEntryPoint, PROFILE, &pVSBlob);
		if (FAILED(hr))
			return hr;

		// Create the vertex shader
		hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, ppVertexShader);
		if (FAILED(hr)) {	
			return hr;
		}

		g_vscache[key] = VertexShaderCacheValue(*ppVertexShader, pVSBlob);
	}

	// Create the input layout
	hr = pDevice->CreateInputLayout(aLayoutDesc, numLayoutDesc, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), ppInputLayout);
	if (FAILED(hr)) {
		(*ppVertexShader)->Release();
		*ppVertexShader = nullptr;
		return hr;
	}

	return S_OK;
}

HRESULT D3DHelper::loadPixelShader(ID3D11Device* pDevice,
								   const TString& strFileName, const char* szEntryPoint,
								   ID3D11PixelShader** ppPixelShader)
{
	const char* PROFILE = "ps_5_0";
	ShaderCacheKey key(strFileName, PROFILE, szEntryPoint);
	auto iter = g_pscache.find(key);
	if (iter != g_pscache.end()) {
		*ppPixelShader = iter->second;
		(*ppPixelShader)->AddRef();
	} else {
		// Compile the pixel shader
		CComPtr<ID3DBlob> pPSBlob = NULL;
		HRESULT hr = compileShader(_T("Shaders/") + strFileName, szEntryPoint, PROFILE, &pPSBlob);
		if (FAILED(hr))
			return hr;

		// Create the pixel shader
		hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, ppPixelShader);
		if (FAILED(hr))
			return hr;

		g_pscache[key] = *ppPixelShader;
	}

	return S_OK;
}

HRESULT D3DHelper::loadHullShader(ID3D11Device* pDevice,
								  const Utils::TString& strFileName, const char* szEntryPoint,
								  ID3D11HullShader** ppHullShader)
{
	const char* PROFILE = "hs_5_0";
	ShaderCacheKey key(strFileName, PROFILE, szEntryPoint);
	auto iter = g_hscache.find(key);
	if (iter != g_hscache.end()) {
		*ppHullShader = iter->second;
		(*ppHullShader)->AddRef();
	} else {
		// Compile the hull shader
		CComPtr<ID3DBlob> pHSBlob = NULL;
		HRESULT hr = compileShader(_T("Shaders/") + strFileName, szEntryPoint, PROFILE, &pHSBlob);
		if (FAILED(hr))
			return hr;

		// Create the hull shader
		hr = pDevice->CreateHullShader(pHSBlob->GetBufferPointer(), pHSBlob->GetBufferSize(), NULL, ppHullShader);
		if (FAILED(hr))
			return hr;

		g_hscache[key] = *ppHullShader;
	}

	return S_OK;
}

HRESULT D3DHelper::loadDomainShader(ID3D11Device* pDevice,
									const Utils::TString& strFileName, const char* szEntryPoint,
									ID3D11DomainShader** ppDomainShader)
{
	const char* PROFILE = "ds_5_0";
	ShaderCacheKey key(strFileName, PROFILE, szEntryPoint);
	auto iter = g_dscache.find(key);
	if (iter != g_dscache.end()) {
		*ppDomainShader = iter->second;
		(*ppDomainShader)->AddRef();
	} else {
		// Compile the hull shader
		CComPtr<ID3DBlob> pDSBlob = NULL;
		HRESULT hr = compileShader(_T("Shaders/") + strFileName, szEntryPoint, PROFILE, &pDSBlob);
		if (FAILED(hr))
			return hr;

		// Create the hull shader
		hr = pDevice->CreateDomainShader(pDSBlob->GetBufferPointer(), pDSBlob->GetBufferSize(), NULL, ppDomainShader);
		if (FAILED(hr))
			return hr;

		g_dscache[key] = *ppDomainShader;
	}
	return S_OK;
}

HRESULT D3DHelper::loadGeometryShader(ID3D11Device* pDevice,
									  const Utils::TString& strFileName, const char* szEntryPoint,
									  ID3D11GeometryShader** ppGeometryShader)
{
	const char* PROFILE = "gs_5_0";
	ShaderCacheKey key(strFileName, PROFILE, szEntryPoint);
	auto iter = g_gscache.find(key);
	if (iter != g_gscache.end()) {
		*ppGeometryShader = iter->second;
		(*ppGeometryShader)->AddRef();
	} else {
		// Compile the hull shader
		CComPtr<ID3DBlob> pGSBlob = NULL;
		HRESULT hr = compileShader(_T("Shaders/") + strFileName, szEntryPoint, PROFILE, &pGSBlob);
		if (FAILED(hr))
			return hr;

		// Create the hull shader
		hr = pDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), NULL, ppGeometryShader);
		if (FAILED(hr))
			return hr;

		g_gscache[key] = *ppGeometryShader;
	}
	return S_OK;
}

HRESULT D3DHelper::createBuffer(ID3D11Device* pDevice, UINT byteWidth, UINT bindFlags, void* pData, ID3D11Buffer** ppBuffer) {
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = byteWidth;
	bd.BindFlags = bindFlags;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = pData;
	HRESULT hr = pDevice->CreateBuffer(&bd, &initData, ppBuffer);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void D3DHelper::getResourceSize(ID3D11ShaderResourceView* pSRV, UINT* pWidth, UINT* pHeight) {
	CComPtr<ID3D11Resource> pResource;
	pSRV->GetResource(&pResource);

	return getResourceSize(pResource, pWidth, pHeight);
}

void D3DHelper::getResourceSize(ID3D11RenderTargetView* pRT, UINT* pWidth, UINT* pHeight) {
	CComPtr<ID3D11Resource> pResource;
	pRT->GetResource(&pResource);

	return getResourceSize(pResource, pWidth, pHeight);
}

void D3DHelper::getResourceSize(ID3D11Resource* pResource, UINT* pWidth, UINT* pHeight) {
	CComQIPtr<ID3D11Texture2D> pT2D = pResource;
	D3D11_TEXTURE2D_DESC desc;
	pT2D->GetDesc(&desc);

	*pWidth = desc.Width;
	*pHeight = desc.Height;
}

HRESULT D3DHelper::loadSRVFromWICFile(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const TString& strFileName,
									  ID3D11ShaderResourceView** ppSRV)
{
	return CreateWICTextureFromFile(pDevice, pDeviceContext, strFileName.c_str(), nullptr, ppSRV);
}

HRESULT D3DHelper::loadSRVFromWICFileEx(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const TString& strFileName,
										size_t maxsize, D3D11_USAGE usage, unsigned int bindFlags, unsigned int cpuAccessFlags,
										unsigned int miscFlags, bool forceSRGB, ID3D11ShaderResourceView** ppSRV)
{
	return CreateWICTextureFromFileEx(pDevice, pDeviceContext, strFileName.c_str(), maxsize, usage, bindFlags, cpuAccessFlags,
		miscFlags, forceSRGB, nullptr, ppSRV);
}

bool D3DHelper::isDDSFile(const TString& strFileName) {
	if (strFileName.length() < 4) return false;
	TString ext = strFileName.substr(strFileName.length() - 4);
	return _tcsicmp(ext.c_str(), _T(".dds")) == 0;
}

HRESULT D3DHelper::loadSRVFromDDSFile(ID3D11Device* pDevice, const TString& strFileName,
									  ID3D11ShaderResourceView** ppSRV, DDS_ALPHA_MODE* pAlphaMode)
{
	return CreateDDSTextureFromFile(pDevice, strFileName.c_str(), nullptr, ppSRV, 0, pAlphaMode);
}

HRESULT D3DHelper::loadSRVFromDDSFileEx(ID3D11Device* pDevice, const TString& strFileName,
										size_t maxsize, D3D11_USAGE usage, unsigned int bindFlags, unsigned int cpuAccessFlags,
										unsigned int miscFlags, bool forceSRGB,
										ID3D11ShaderResourceView** ppSRV, DDS_ALPHA_MODE* pAlphaMode)
{
	return CreateDDSTextureFromFileEx(pDevice, strFileName.c_str(), 0, usage, bindFlags, cpuAccessFlags, miscFlags, forceSRGB,
		nullptr, ppSRV, pAlphaMode);
}

HRESULT D3DHelper::loadSRVFromMemory(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, void* pData, UINT width, UINT height, DXGI_FORMAT format,
									 UINT rowPitch, ID3D11ShaderResourceView** ppSRV, bool autogen)
{
	// Create texture
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = (autogen) ? 0 : 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = (autogen) ? (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET) : (D3D11_BIND_SHADER_RESOURCE);
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = (autogen) ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = pData;
	initData.SysMemPitch = rowPitch;
	initData.SysMemSlicePitch = 0;

	CComPtr<ID3D11Texture2D> pTexture2D;
	HRESULT hr = pDevice->CreateTexture2D(&desc, (autogen) ? nullptr : &initData, &pTexture2D);
	if (SUCCEEDED(hr) && pTexture2D != 0) {
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		memset(&SRVDesc, 0, sizeof(SRVDesc));
		SRVDesc.Format = format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = (autogen) ? -1 : 1;

		hr = pDevice->CreateShaderResourceView(pTexture2D, &SRVDesc, ppSRV);
		if (FAILED(hr)) {
			return hr;
		}

		if (autogen) {
			ASSERT(pDeviceContext != 0);
			pDeviceContext->UpdateSubresource(pTexture2D, 0, nullptr, pData, rowPitch, 0);
			pDeviceContext->GenerateMips(*ppSRV);
		}
	}

	return hr;
}

namespace Skin { namespace D3DHelper {
	static size_t _WICBitsPerPixel( REFGUID targetGuid ) {
		IWICImagingFactory* pWIC = GetWIC();
		if ( !pWIC )
			return 0;
 
		CComPtr<IWICComponentInfo> cinfo;
		if ( FAILED( pWIC->CreateComponentInfo( targetGuid, &cinfo ) ) )
			return 0;

		WICComponentType type;
		if ( FAILED( cinfo->GetComponentType( &type ) ) )
			return 0;

		if ( type != WICPixelFormat )
			return 0;

		CComPtr<IWICPixelFormatInfo> pfinfo;
		if ( FAILED( cinfo->QueryInterface( __uuidof(IWICPixelFormatInfo), reinterpret_cast<void**>( &pfinfo )  ) ) )
			return 0;

		UINT bpp;
		if ( FAILED( pfinfo->GetBitsPerPixel( &bpp ) ) )
			return 0;

		return bpp;
	}

	static size_t _WICBytesPerPixel( REFGUID targetGuid ) {
		return (_WICBitsPerPixel(targetGuid) + 7) / 8;
	}
} } // namespace Skin::D3DHelper

HRESULT D3DHelper::loadImageData(const TString& strFileName, void** ppData, UINT* pWidth, UINT* pHeight,
								 WICPixelFormatGUID* pPixelFormatGUID)
{
	if (*ppData)
		return E_INVALIDARG;

	IWICImagingFactory* pWIC = GetWIC();
	if (!pWIC)
		return E_NOINTERFACE;

	// Initialize WIC
	CComPtr<IWICBitmapDecoder> decoder;
	HRESULT hr = pWIC->CreateDecoderFromFilename(strFileName.c_str(), 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
	if (FAILED(hr))
		return hr;

	CComPtr<IWICBitmapFrameDecode> frame;
	hr = decoder->GetFrame(0, &frame);
	if (FAILED(hr))
		return hr;

	frame->GetSize(pWidth, pHeight);
	frame->GetPixelFormat(pPixelFormatGUID);
	UINT bytePP = _WICBytesPerPixel(*pPixelFormatGUID);
	UINT stride = (*pWidth) * bytePP;
	UINT size = (*pWidth) * (*pHeight) * bytePP;
	*ppData = malloc(size);

	hr = frame->CopyPixels(nullptr, stride, size, (BYTE*)(*ppData));
	if (FAILED(hr)) {
		delete [] *ppData;
		*ppData = nullptr;
		return hr;
	}

	return S_OK;
}

void D3DHelper::freeImageData(void* pData) {
	free(pData);
}

HRESULT D3DHelper::createSamplerState(ID3D11Device* pDevice, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressU,
									  D3D11_TEXTURE_ADDRESS_MODE addressV, D3D11_TEXTURE_ADDRESS_MODE addressW, ID3D11SamplerState** ppSamplerState)
{
	return createSamplerComparisonState(pDevice, filter, addressU, addressV, addressW, D3D11_COMPARISON_NEVER, ppSamplerState);
}

HRESULT D3DHelper::createSamplerComparisonState(ID3D11Device* pDevice, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressU,
												D3D11_TEXTURE_ADDRESS_MODE addressV, D3D11_TEXTURE_ADDRESS_MODE addressW, D3D11_COMPARISON_FUNC comp,
												ID3D11SamplerState** ppSamplerState)
{
	return createSamplerComparisonStateEx(pDevice, filter, addressU, addressV, addressW, comp, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f), 0, ppSamplerState);
}

HRESULT D3DHelper::createSamplerStateEx(ID3D11Device* pDevice, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressU,
										D3D11_TEXTURE_ADDRESS_MODE addressV, D3D11_TEXTURE_ADDRESS_MODE addressW,
										XMFLOAT4 borderColor, UINT maxAnisotropy, ID3D11SamplerState** ppSamplerState)
{
	return createSamplerComparisonStateEx(pDevice, filter, addressU, addressV, addressW, D3D11_COMPARISON_NEVER,
		borderColor, maxAnisotropy, ppSamplerState);
}

HRESULT D3DHelper::createSamplerComparisonStateEx(ID3D11Device* pDevice, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressU,
												  D3D11_TEXTURE_ADDRESS_MODE addressV, D3D11_TEXTURE_ADDRESS_MODE addressW, D3D11_COMPARISON_FUNC comp,
												  XMFLOAT4 borderColor, UINT maxAnisotropy, ID3D11SamplerState** ppSamplerState)
{
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory( &sampDesc, sizeof(sampDesc) );
	sampDesc.Filter = filter;
	sampDesc.AddressU = addressU;
	sampDesc.AddressV = addressV;
	sampDesc.AddressW = addressW;
	sampDesc.ComparisonFunc = comp;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	sampDesc.MaxAnisotropy = maxAnisotropy;
	memcpy(sampDesc.BorderColor, &borderColor, sizeof(sampDesc.BorderColor));
	HRESULT hr = pDevice->CreateSamplerState(&sampDesc, ppSamplerState);
	if (FAILED(hr))
		return hr;
	return S_OK;
}

HRESULT D3DHelper::createTexture2D(ID3D11Device* pDevice, UINT width, UINT height, DXGI_FORMAT format,
								   ID3D11Texture2D** ppTexture2D, D3D11_BIND_FLAG bindFlags)
{
	return createTexture2DEx(pDevice, width, height, format, true, 1, 0, ppTexture2D, bindFlags);
}

HRESULT D3DHelper::createTexture2DEx(ID3D11Device* pDevice, UINT width, UINT height, DXGI_FORMAT format,
									 bool multisampled, UINT multisampleCount, UINT multisampleQuality,
									 ID3D11Texture2D** ppTexture2D, D3D11_BIND_FLAG bindFlags)
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = multisampled ? 1 : 0;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = multisampled ? multisampleCount : 1;
	desc.SampleDesc.Quality = multisampled ? multisampleQuality : 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = bindFlags;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = multisampled ? 0 : D3D11_RESOURCE_MISC_GENERATE_MIPS;

	HRESULT hr = pDevice->CreateTexture2D(&desc, nullptr, ppTexture2D);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT D3DHelper::createSRVFromTexture2D(ID3D11Device* pDevice, ID3D11Texture2D* pTexture2D, DXGI_FORMAT format,
										  ID3D11ShaderResourceView** ppShaderResourceView)
{
	return createSRVFromTexture2DEx(pDevice, pTexture2D, format, false, ppShaderResourceView);
}

HRESULT D3DHelper::createSRVFromTexture2DEx(ID3D11Device* pDevice, ID3D11Texture2D* pTexture2D, DXGI_FORMAT format, bool multisampled,
											ID3D11ShaderResourceView** ppShaderResourceView)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = format;
	srvDesc.ViewDimension = multisampled ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	HRESULT hr = pDevice->CreateShaderResourceView(pTexture2D, &srvDesc, ppShaderResourceView);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT D3DHelper::createRTFromTexture2D(ID3D11Device* pDevice, ID3D11Texture2D* pTexture2D, DXGI_FORMAT format,
										 ID3D11RenderTargetView** ppRenderTargetView)
{
	return createRTFromTexture2DEx(pDevice, pTexture2D, format, false, ppRenderTargetView);
}

HRESULT D3DHelper::createRTFromTexture2DEx(ID3D11Device* pDevice, ID3D11Texture2D* pTexture2D, DXGI_FORMAT format, bool multisampled,
										   ID3D11RenderTargetView** ppRenderTargetView)
{
	D3D11_RENDER_TARGET_VIEW_DESC desc;
	desc.Format = format;
	desc.ViewDimension = multisampled ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipSlice = 0;

	HRESULT hr = pDevice->CreateRenderTargetView(pTexture2D, &desc, ppRenderTargetView);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT D3DHelper::createShaderResourceView2D(ID3D11Device* pDevice, UINT width, UINT height, DXGI_FORMAT format,
											  ID3D11ShaderResourceView** ppTextureResourceView, D3D11_BIND_FLAG bindFlags)
{
	CComPtr<ID3D11Texture2D> pTexture2D;
	HRESULT hr = createTexture2D(pDevice, width, height, format, &pTexture2D, bindFlags);
	if (FAILED(hr))
		return hr;

	hr = createSRVFromTexture2D(pDevice, pTexture2D, format, ppTextureResourceView);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT D3DHelper::createIntermediateRenderTarget(ID3D11Device* pDevice, UINT width, UINT height, DXGI_FORMAT format,
												  ID3D11Texture2D** ppTexture2D, ID3D11ShaderResourceView** ppShaderResourceView,
												  ID3D11RenderTargetView** ppRenderTargetView)
{
	return createIntermediateRenderTargetEx(pDevice, width, height, format, true, 1, 0, ppTexture2D, ppShaderResourceView, ppRenderTargetView);
}

HRESULT D3DHelper::createIntermediateRenderTargetEx(ID3D11Device* pDevice, UINT width, UINT height, DXGI_FORMAT format,
													bool multisampled, UINT multisampleCount, UINT multisampleQuality,
													ID3D11Texture2D** ppTexture2D, ID3D11ShaderResourceView** ppShaderResourceView,
													ID3D11RenderTargetView** ppRenderTargetView)
{
	if (!ppTexture2D && !ppShaderResourceView && !ppRenderTargetView)
		return E_INVALIDARG;

	CComPtr<ID3D11Texture2D> pTexture2D;
	HRESULT hr = createTexture2DEx(pDevice, width, height, format, multisampled, multisampleCount, multisampleQuality,
		&pTexture2D, (D3D11_BIND_FLAG)(D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET));
	if (FAILED(hr))
		return hr;

	CComPtr<ID3D11ShaderResourceView> pShaderResourceView;
	if (ppShaderResourceView) {
		hr = createSRVFromTexture2DEx(pDevice, pTexture2D, format, multisampled && multisampleCount > 1, &pShaderResourceView);
		if (FAILED(hr))
			return hr;
	}

	CComPtr<ID3D11RenderTargetView> pRenderTargetView;
	if (ppRenderTargetView) {
		hr = createRTFromTexture2DEx(pDevice, pTexture2D, format, multisampled && multisampleCount > 1, &pRenderTargetView);
		if (FAILED(hr))
			return hr;
	}

	if (ppTexture2D) {
		*ppTexture2D = pTexture2D;
		(*ppTexture2D)->AddRef();
	}
	if (ppShaderResourceView) {
		*ppShaderResourceView = pShaderResourceView;
		(*ppShaderResourceView)->AddRef();
	}
	if (ppRenderTargetView) {
		*ppRenderTargetView = pRenderTargetView;
		(*ppRenderTargetView)->AddRef();
	}

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Extract all 6 plane equations from frustum denoted by supplied matrix
//--------------------------------------------------------------------------------------
void D3DHelper::extractPlanesFromFrustum(XMFLOAT4 aPlaneEquation[6], const XMMATRIX& matrix, bool bNormalize) {
	// Left clipping plane
	aPlaneEquation[0].x = matrix._14 + matrix._11;
	aPlaneEquation[0].y = matrix._24 + matrix._21;
	aPlaneEquation[0].z = matrix._34 + matrix._31;
	aPlaneEquation[0].w = matrix._44 + matrix._41;

	// Right clipping plane
	aPlaneEquation[1].x = matrix._14 - matrix._11;
	aPlaneEquation[1].y = matrix._24 - matrix._21;
	aPlaneEquation[1].z = matrix._34 - matrix._31;
	aPlaneEquation[1].w = matrix._44 - matrix._41;

	// Top clipping plane
	aPlaneEquation[2].x = matrix._14 - matrix._12;
	aPlaneEquation[2].y = matrix._24 - matrix._22;
	aPlaneEquation[2].z = matrix._34 - matrix._32;
	aPlaneEquation[2].w = matrix._44 - matrix._42;

	// Bottom clipping plane
	aPlaneEquation[3].x = matrix._14 + matrix._12;
	aPlaneEquation[3].y = matrix._24 + matrix._22;
	aPlaneEquation[3].z = matrix._34 + matrix._32;
	aPlaneEquation[3].w = matrix._44 + matrix._42;

	// Near clipping plane
	aPlaneEquation[4].x = matrix._13;
	aPlaneEquation[4].y = matrix._23;
	aPlaneEquation[4].z = matrix._33;
	aPlaneEquation[4].w = matrix._43;

	// Far clipping plane
	aPlaneEquation[5].x = matrix._14 - matrix._13;
	aPlaneEquation[5].y = matrix._24 - matrix._23;
	aPlaneEquation[5].z = matrix._34 - matrix._33;
	aPlaneEquation[5].w = matrix._44 - matrix._43;

	// Normalize the plane equations, if requested
	if (bNormalize) {
		normalizePlane(aPlaneEquation[0]);
		normalizePlane(aPlaneEquation[1]);
		normalizePlane(aPlaneEquation[2]);
		normalizePlane(aPlaneEquation[3]);
		normalizePlane(aPlaneEquation[4]);
		normalizePlane(aPlaneEquation[5]);
	}
}

//--------------------------------------------------------------------------------------
// Helper function to normalize a plane
//--------------------------------------------------------------------------------------
void D3DHelper::normalizePlane(XMFLOAT4& planeEquation) {
	float mag;

	mag = sqrt( planeEquation.x * planeEquation.x + 
		planeEquation.y * planeEquation.y + 
		planeEquation.z * planeEquation.z );

	planeEquation.x = planeEquation.x / mag;
	planeEquation.y = planeEquation.y / mag;
	planeEquation.z = planeEquation.z / mag;
	planeEquation.w = planeEquation.w / mag;
}

D3D11_VIEWPORT D3DHelper::createViewport(UINT width, UINT height, UINT topLeftX, UINT topLeftY) {
	D3D11_VIEWPORT vp;
	vp.Width = (float)width;
	vp.Height = (float)height;
	vp.TopLeftX = (float)topLeftX;
	vp.TopLeftY = (float)topLeftY;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	return vp;
}
