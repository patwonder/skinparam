/**
 * Main renderer class
 */

#include "stdafx.h"

#include "Renderer.h"
#include "Config.h"
#include "FVector.h"

using namespace Skin;
using namespace Utils;

namespace Skin {
	inline D3DXVECTOR3 DxVec(const Vector& v) {
		return D3DXVECTOR3((float)v.x, (float)v.y, (float)v.z);
	}
}

Renderer::Renderer(HWND hwnd, CRect rectView, Config* pConfig, Camera* pCamera)
	: m_pDevice(nullptr),
	  m_pDeviceContext(nullptr),
	  m_pSwapChain(nullptr),
	  m_pRenderTargetView(nullptr),
	  m_hwnd(hwnd),
	  m_rectView(rectView),
	  m_pConfig(pConfig),
	  m_pCamera(pCamera),
	  m_nFrameCount(0),
	  m_nStartTick(GetTickCount()),
	  m_fps(0.0f)
{
	m_vppCOMObjs.push_back((IUnknown**)&m_pDevice);
	m_vppCOMObjs.push_back((IUnknown**)&m_pDeviceContext);
	m_vppCOMObjs.push_back((IUnknown**)&m_pSwapChain);
	m_vppCOMObjs.push_back((IUnknown**)&m_pRenderTargetView);

	checkFailure(initDX(), _T("Failed to initialize DirectX 11"));
}

Renderer::~Renderer() {
	removeAllRenderables();

	if (m_pDeviceContext)
		m_pDeviceContext->ClearState();

	for (IUnknown** ppCOMObj: m_vppCOMObjs) {
		if (*ppCOMObj)
			(*ppCOMObj)->Release();
		*ppCOMObj = nullptr;
	}

	m_vppCOMObjs.clear();
}

void Renderer::checkFailure(HRESULT hr, const TString& prompt) {
	if (FAILED(hr)) {
		TStringStream ss;
		ss << (prompt) << _T("\nError description: ") << DXGetErrorDescription(hr);
		::MessageBox(NULL, ss.str().c_str(), _T("Fatal Error"), MB_OK | MB_ICONERROR);
		::exit(EXIT_FAILURE);
	}
}

HRESULT Renderer::initDX() {
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
	sd.BufferDesc.Width = m_rectView.Width();
	sd.BufferDesc.Height = m_rectView.Height();
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

	HRESULT hr;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
		m_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(NULL, m_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pDevice, &m_featureLevel, &m_pDeviceContext);
        if (SUCCEEDED(hr))
            break;
    }
    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = NULL;
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return hr;

	hr = m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);

    // Setup the viewport
    D3D11_VIEWPORT vp;
	vp.Width = (float)m_rectView.Width();
	vp.Height = (float)m_rectView.Height();
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
	vp.TopLeftX = (float)m_rectView.left;
	vp.TopLeftY = (float)m_rectView.top;
    m_pDeviceContext->RSSetViewports(1, &vp);

    return S_OK;
}


struct SimpleVertex {
	FVector pos;
};

void Renderer::addRenderable(Renderable* renderable) {
	renderable->init(m_pDevice);
	m_vpRenderables.push_back(renderable);
}

void Renderer::removeRenderable(Renderable* renderable) {
	for (size_t i = 0; i < m_vpRenderables.size(); i++) {
		if (m_vpRenderables[i] == renderable) {
			m_vpRenderables.erase(m_vpRenderables.begin() + i);
			renderable->cleanup();
			break;
		}
	}
}

void Renderer::removeAllRenderables() {
	for (Renderable* pRenderable : m_vpRenderables) {
		pRenderable->cleanup();
	}
	m_vpRenderables.clear();
}

void Renderer::render() {
    //
    // Clear the backbuffer
    //
    float ClearColor[4] = { 0.8f, 0.4f, 0.0f, 1.0f }; // RGBA
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, ClearColor);
    
	renderScene();

	m_pSwapChain->Present(m_pConfig->vsync ? 1 : 0, 0);

	computeStats();
}

void Renderer::renderScene() {
	for (Renderable* renderable : m_vpRenderables) {
		renderable->render(m_pDeviceContext, *m_pCamera);
	}
}

void Renderer::computeStats() {
	m_nFrameCount++;
	DWORD tick = GetTickCount();
	if (tick - m_nStartTick >= 1000) {
		DWORD remainder = /*(tick - m_nStartTick) % 1000*/0;
		m_fps = (float)(m_nFrameCount * 1000) / (tick - m_nStartTick - remainder);
		m_nStartTick = tick - remainder;
		m_nFrameCount = 0;
	}
}
