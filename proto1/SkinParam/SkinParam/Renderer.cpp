/**
 * Main renderer class
 */

#include "stdafx.h"

#include "Renderer.h"
#include "Config.h"
#include "FVector.h"
#include "ShaderGroup.h"

using namespace Skin;
using namespace Utils;
using namespace D3DHelper;

namespace Skin {
	inline XMVECTOR XMVec(const Vector& v) {
		return XMVectorSet((float)v.x, (float)v.y, (float)v.z, 0.0f);
	}
}

Renderer::Renderer(HWND hwnd, CRect rectView, Config* pConfig, Camera* pCamera)
	: m_pDevice(nullptr),
	  m_pDeviceContext(nullptr),
	  m_pSwapChain(nullptr),
	  m_pRenderTargetView(nullptr),
	  m_pTransformConstantBuffer(nullptr),
	  m_psgTriangle(nullptr),
	  m_psgPhong(nullptr),
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
	m_vppCOMObjs.push_back((IUnknown**)&m_pTransformConstantBuffer);

	checkFailure(initDX(), _T("Failed to initialize DirectX 11"));

	loadShaders();
	initTransform();
	updateProjection();
}

Renderer::~Renderer() {
	removeAllRenderables();
	unloadShaders();

	if (m_pDeviceContext)
		m_pDeviceContext->ClearState();

	for (IUnknown** ppCOMObj: m_vppCOMObjs) {
		if (*ppCOMObj)
			(*ppCOMObj)->Release();
		*ppCOMObj = nullptr;
	}

	m_vppCOMObjs.clear();
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


void Renderer::loadShaders() {
    D3D11_INPUT_ELEMENT_DESC tri_layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	m_psgTriangle = new ShaderGroup(m_pDevice, _T("Triangle.fx"), tri_layout, ARRAYSIZE(tri_layout), "VS", "PS");

	D3D11_INPUT_ELEMENT_DESC phong_layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	m_psgPhong = new ShaderGroup(m_pDevice, _T("Phong.fx"), phong_layout, ARRAYSIZE(phong_layout), "VS", "PS");

}

void Renderer::unloadShaders() {
	delete m_psgTriangle; m_psgTriangle = nullptr;
	delete m_psgPhong; m_psgPhong = nullptr;
}

void Renderer::initTransform() {
	m_cbTransform.g_matWorld = XMMatrixIdentity();
	m_cbTransform.g_matViewProj = XMMatrixIdentity();
	checkFailure(createConstantBuffer(m_pDevice, &m_cbTransform, &m_pTransformConstantBuffer),
		_T("Failed to create constant buffer"));
}

void Renderer::updateTransform() {
	Vector vEye, vLookAt, vUp;
	m_pCamera->look(vEye, vLookAt, vUp);

	XMMATRIX matView = XMMatrixLookAtLH(XMVec(vEye), XMVec(vLookAt), XMVec(vUp));
	XMMATRIX matViewProj = XMMatrixMultiply(matView, XMLoadFloat4x4(&m_matProjection));
	m_cbTransform.g_matViewProj = XMMatrixTranspose(matViewProj);

	m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pTransformConstantBuffer);
}

void Renderer::updateProjection() {
	XMStoreFloat4x4(&m_matProjection,
		XMMatrixPerspectiveFovLH((float)(D3DX_PI / 4), (float)m_rectView.Width() / m_rectView.Height(), 0.1f, 10000.0f));
}

void Renderer::render() {
    //
    // Clear the backbuffer
    //
    //float ClearColor[4] = { 0.8f, 0.4f, 0.0f, 1.0f }; // RGBA
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // RGBA
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, ClearColor);
    
	m_psgPhong->use(m_pDeviceContext);
	updateTransform();

	renderScene();

	m_pSwapChain->Present(m_pConfig->vsync ? 1 : 0, 0);

	computeStats();
}

void Renderer::renderScene() {
	for (Renderable* renderable : m_vpRenderables) {
		if (renderable->useTransform()) {
			m_cbTransform.g_matWorld = XMMatrixTranspose(renderable->getWorldMatrix());
			m_pDeviceContext->UpdateSubresource(m_pTransformConstantBuffer, 0, NULL, &m_cbTransform, 0, 0);
		}
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
