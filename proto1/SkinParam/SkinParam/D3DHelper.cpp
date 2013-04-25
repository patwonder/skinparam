/**
 * D3D helper functions
 */

#include "stdafx.h"

#include "D3DHelper.h"

using namespace Skin;
using namespace Utils;

void D3DHelper::checkFailure(HRESULT hr, const TString& prompt) {
	if (FAILED(hr)) {
		TStringStream ss;
		ss << (prompt) << _T("\nError description: ") << DXGetErrorDescription(hr);
		::MessageBox(NULL, ss.str().c_str(), _T("Fatal Error"), MB_OK | MB_ICONERROR);
		::exit(EXIT_FAILURE);
	}
}

HRESULT D3DHelper::compileShader(const TString& strFileName, const char* szEntryPoint, const char* szShaderModel, ID3DBlob** ppBlobOut) {
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile(strFileName.c_str(), NULL, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
    if (FAILED(hr)) {
        if( pErrorBlob != NULL )
            MessageBoxA(NULL, (char*)pErrorBlob->GetBufferPointer(), "Error Compiling Shader", MB_OK | MB_ICONERROR);
        if( pErrorBlob ) pErrorBlob->Release();
        return hr;
    }
    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT D3DHelper::loadVertexShaderAndLayout(ID3D11Device* pDevice,
	const TString& strFileName, const char* szEntryPoint,
	D3D11_INPUT_ELEMENT_DESC aLayoutDesc[], UINT numLayoutDesc,
	ID3D11VertexShader** ppVertexShader, ID3D11InputLayout** ppInputLayout)
{
    // Compile the vertex shader
    ID3DBlob* pVSBlob = NULL;
    HRESULT hr = compileShader(_T("Shaders/") + strFileName, szEntryPoint, "vs_4_0", &pVSBlob);
    if (FAILED(hr))
        return hr;

	// Create the vertex shader
	hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, ppVertexShader);
	if (FAILED(hr)) {	
		pVSBlob->Release();
        return hr;
	}
   
    // Create the input layout
	hr = pDevice->CreateInputLayout(aLayoutDesc, numLayoutDesc, pVSBlob->GetBufferPointer(),
                                    pVSBlob->GetBufferSize(), ppInputLayout);
	pVSBlob->Release();
	if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT D3DHelper::loadPixelShader(ID3D11Device* pDevice,
	const TString& strFileName, const char* szEntryPoint,
	ID3D11PixelShader** ppPixelShader)
{
	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
    HRESULT hr = compileShader(_T("Shaders/") + strFileName, szEntryPoint, "ps_4_0", &pPSBlob);
    if (FAILED(hr))
        return hr;

	// Create the pixel shader
	hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, ppPixelShader);
	pPSBlob->Release();
    if (FAILED(hr))
        return hr;

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

HRESULT D3DHelper::loadTexture(ID3D11Device* pDevice, const Utils::TString& strFileName, ID3D11ShaderResourceView** ppTexture) {
	return D3DX11CreateShaderResourceViewFromFile(pDevice, strFileName.c_str(), nullptr, nullptr, ppTexture, nullptr);
}

HRESULT D3DHelper::createSamplerState(ID3D11Device* pDevice, D3D11_FILTER filer, D3D11_TEXTURE_ADDRESS_MODE addressU,
	D3D11_TEXTURE_ADDRESS_MODE addressV, D3D11_TEXTURE_ADDRESS_MODE addressW, ID3D11SamplerState** ppSamplerState)
{
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.Filter = filer;
    sampDesc.AddressU = addressU;
    sampDesc.AddressV = addressV;
    sampDesc.AddressW = addressW;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    HRESULT hr = pDevice->CreateSamplerState(&sampDesc, ppSamplerState);
    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT D3DHelper::createTexture2D(ID3D11Device* pDevice, UINT width, UINT height, DXGI_FORMAT format,
								   ID3D11ShaderResourceView** ppTexture)
{
    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

	ID3D11Texture2D* pTexture2D = nullptr;
    HRESULT hr = pDevice->CreateTexture2D(&desc, nullptr, &pTexture2D);
    if (FAILED(hr))
        return hr;


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	hr = pDevice->CreateShaderResourceView(pTexture2D, &srvDesc, ppTexture);
	pTexture2D->Release();
	if (FAILED(hr))
		return hr;

	return S_OK;
}
