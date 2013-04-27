/**
 * Main renderer class
 */

#include "stdafx.h"

#include "Renderer.h"
#include "Config.h"
#include "ShaderGroup.h"
#include "Light.h"
#include "Material.h"

#include "FVector.h"

using namespace Skin;
using namespace Utils;
using namespace D3DHelper;

namespace Skin {
	inline XMVECTOR XMVec(const Vector& v) {
		return XMVectorSet((float)v.x, (float)v.y, (float)v.z, 0.0f);
	}
	inline XMFLOAT4 XMColor(const Color& c) {
		return XMFLOAT4(c.red, c.green, c.blue, c.alpha);
	}
	inline XMFLOAT3 XMColor3(const Color& c) {
		return XMFLOAT3(c.red, c.green, c.blue);
	}
	inline XMFLOAT3 XMPos(const Vector& p) {
		return XMFLOAT3((float)p.x, (float)p.y, (float)p.z);
	}
}

Renderer::Renderer(HWND hwnd, CRect rectView, Config* pConfig, Camera* pCamera)
	: m_pDevice(nullptr),
	  m_pDeviceContext(nullptr),
	  m_pSwapChain(nullptr),
	  m_pRenderTargetView(nullptr),
	  m_pDepthStencil(nullptr),
	  m_pDepthStencilView(nullptr),
	  m_pPlaceholderSamplerState(nullptr),
	  m_pPlaceholderTexture(nullptr),
	  m_pTransformConstantBuffer(nullptr),
	  m_pLightingConstantBuffer(nullptr),
	  m_pMaterialConstantBuffer(nullptr),
	  m_psgTriangle(nullptr),
	  m_psgPhong(nullptr),
	  m_psgTesselatedPhong(nullptr),
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
	m_vppCOMObjs.push_back((IUnknown**)&m_pDepthStencil);
	m_vppCOMObjs.push_back((IUnknown**)&m_pDepthStencilView);
	m_vppCOMObjs.push_back((IUnknown**)&m_pTransformConstantBuffer);
	m_vppCOMObjs.push_back((IUnknown**)&m_pLightingConstantBuffer);
	m_vppCOMObjs.push_back((IUnknown**)&m_pMaterialConstantBuffer);
	m_vppCOMObjs.push_back((IUnknown**)&m_pPlaceholderSamplerState);
	m_vppCOMObjs.push_back((IUnknown**)&m_pPlaceholderTexture);

	checkFailure(initDX(), _T("Failed to initialize DirectX 11"));

	initMisc();

	loadShaders();
	initTransform();
	initLighting();
	initMaterial();
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
        //D3D_DRIVER_TYPE_HARDWARE,
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
        hr = D3D11CreateDeviceAndSwapChain(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pDevice, &m_featureLevel, &m_pDeviceContext);
        if (SUCCEEDED(hr))
            break;
    }
    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return hr;

	hr = m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = m_rectView.Width();
	descDepth.Height = m_rectView.Height();
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = m_pDevice->CreateTexture2D(&descDepth, nullptr, &m_pDepthStencil);
    if (FAILED(hr))
        return hr;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = m_pDevice->CreateDepthStencilView(m_pDepthStencil, &descDSV, &m_pDepthStencilView);
    if (FAILED(hr))
        return hr;

	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	// Setup the viewport
    D3D11_VIEWPORT vp;
	vp.Width = (float)m_rectView.Width();
	vp.Height = (float)m_rectView.Height();
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
	vp.TopLeftX = (float)m_rectView.left;
	vp.TopLeftY = (float)m_rectView.top;
    m_pDeviceContext->RSSetViewports(1, &vp);

	// Create default rasterizer state
	D3D11_RASTERIZER_DESC& desc = m_descRasterizerState;
	ZeroMemory(&desc, sizeof(desc));
	desc.FillMode = D3D11_FILL_SOLID;
	desc.CullMode = D3D11_CULL_BACK;
	desc.DepthClipEnable = TRUE;

	// Init togglable states
	m_bTessellation = true;

    return S_OK;
}

void Renderer::initMisc() {
	// Creates the placeholder texture for non-textured objects
	checkFailure(createSamplerState(m_pDevice, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, &m_pPlaceholderSamplerState),
		_T("Failed to create placeholder sampler state"));

	checkFailure(createTexture2D(m_pDevice, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, &m_pPlaceholderTexture),
		_T("Failed to create placeholder texture"));

	float initData[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	ID3D11Resource* pResource = nullptr;
	m_pPlaceholderTexture->GetResource(&pResource);
	m_pDeviceContext->UpdateSubresource(pResource, 0, nullptr, initData, 16, 0);
	pResource->Release();
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

void Renderer::addLight(Light* light) {
	m_vpLights.push_back(light);
}

void Renderer::removeLight(Light* light) {
	for (size_t i = 0; i < m_vpLights.size(); i++) {
		if (m_vpLights[i] == light) {
			m_vpLights.erase(m_vpLights.begin() + i);
			break;
		}
	}
}

void Renderer::removeAllLights() {
	m_vpLights.clear();
}

void Renderer::loadShaders() {
    D3D11_INPUT_ELEMENT_DESC tri_layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	m_psgTriangle = new ShaderGroup(m_pDevice, _T("Triangle.fx"), tri_layout, ARRAYSIZE(tri_layout), "VS", nullptr, nullptr, "PS");

	D3D11_INPUT_ELEMENT_DESC phong_layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	m_psgPhong = new ShaderGroup(m_pDevice, _T("Phong.fx"), phong_layout, ARRAYSIZE(phong_layout), "VS", nullptr, nullptr, "PS");

	m_psgTesselatedPhong = new ShaderGroup(m_pDevice, _T("TessellatedPhong.fx"), phong_layout, ARRAYSIZE(phong_layout),
		"VS", "HS", "DS", "PS");
}

void Renderer::unloadShaders() {
	delete m_psgTriangle; m_psgTriangle = nullptr;
	delete m_psgPhong; m_psgPhong = nullptr;
	delete m_psgTesselatedPhong; m_psgTesselatedPhong = nullptr;
}

void Renderer::initTransform() {
	XMStoreFloat4x4(&m_cbTransform.g_matWorld, XMMatrixIdentity());
	XMStoreFloat4x4(&m_cbTransform.g_matViewProj, XMMatrixIdentity());
	m_cbTransform.g_posEye = XMPos(Vector::ZERO);
	checkFailure(createConstantBuffer(m_pDevice, &m_cbTransform, &m_pTransformConstantBuffer),
		_T("Failed to create transform constant buffer"));
}

void Renderer::updateTransform() {
	Vector vEye, vLookAt, vUp;
	m_pCamera->look(vEye, vLookAt, vUp);

	XMMATRIX matView = XMMatrixLookAtLH(XMVec(vEye), XMVec(vLookAt), XMVec(vUp));
	XMMATRIX matViewProj = XMMatrixMultiply(matView, XMLoadFloat4x4(&m_matProjection));
	XMStoreFloat4x4(&m_cbTransform.g_matViewProj, XMMatrixTranspose(matViewProj));

	m_cbTransform.g_posEye = XMPos(vEye);
}

void Renderer::setGlobalAmbient(const Color& coAmbient) {
	m_cbLighting.g_ambient = XMColor3(coAmbient);
}

void Renderer::initLighting() {
	m_cbLighting.g_ambient = XMColor3(Color::Black);
	for (UINT i = 0; i < LightingConstantBuffer::NUM_LIGHTS; i++) {
		RLight& l = m_cbLighting.g_lights[i];
		l.ambient = XMColor3(Color::Black);
		l.diffuse = XMColor3(Color::Black);
		l.specular = XMColor3(Color::Black);
		l.position = XMColor3(Color::Black);
		l.attenuation = XMFLOAT3(1.0f, 0.0f, 0.0f);
	}

	checkFailure(createConstantBuffer(m_pDevice, &m_cbLighting, &m_pLightingConstantBuffer),
		_T("Failed to create lighting constant buffer"));
}

void Renderer::updateLighting() {
	for (UINT i = 0; i < LightingConstantBuffer::NUM_LIGHTS && i < m_vpLights.size(); i++) {
		RLight& rl = m_cbLighting.g_lights[i];
		const Light* l = m_vpLights[i];
		rl.ambient = XMColor3(l->coAmbient);
		rl.diffuse = XMColor3(l->coDiffuse);
		rl.specular = XMColor3(l->coSpecular);
		rl.position = XMPos(l->vecPosition);
		rl.attenuation = XMFLOAT3(l->fAtten0, l->fAtten1, l->fAtten2);
	}
	for (UINT i = m_vpLights.size(); i < LightingConstantBuffer::NUM_LIGHTS; i++) {
		RLight& l = m_cbLighting.g_lights[i];
		l.ambient = XMColor3(Color::Black);
		l.diffuse = XMColor3(Color::Black);
		l.specular = XMColor3(Color::Black);
		l.position = XMPos(Vector::ZERO);
		l.attenuation = XMFLOAT3(1.0f, 0.0f, 0.0f);
	}
	m_pDeviceContext->UpdateSubresource(m_pLightingConstantBuffer, 0, NULL, &m_cbLighting, 0, 0);
}

void Renderer::initMaterial() {
	m_cbMaterial.g_mtAmbient = XMColor3(Color::White);
	m_cbMaterial.g_mtDiffuse = XMColor3(Color::White);
	m_cbMaterial.g_mtSpecular = XMColor3(Color::White);
	m_cbMaterial.g_mtEmissive = XMColor3(Color::Black);
	m_cbMaterial.g_mtShininess = 0.0f;

	checkFailure(createConstantBuffer(m_pDevice, &m_cbMaterial, &m_pMaterialConstantBuffer),
		_T("Failed to create material constant buffer"));
}

void Renderer::updateProjection() {
	XMStoreFloat4x4(&m_matProjection,
		XMMatrixPerspectiveFovLH((float)(Math::PI / 4), (float)m_rectView.Width() / m_rectView.Height(), 0.1f, 10000.0f));
}

void Renderer::setConstantBuffers() {
	ID3D11Buffer* buffers[] = {
		m_pTransformConstantBuffer,
		m_pLightingConstantBuffer,
		m_pMaterialConstantBuffer
	};

	m_pDeviceContext->VSSetConstantBuffers(0, ARRAYSIZE(buffers), buffers);
	m_pDeviceContext->HSSetConstantBuffers(0, ARRAYSIZE(buffers), buffers);
	m_pDeviceContext->DSSetConstantBuffers(0, ARRAYSIZE(buffers), buffers);
	m_pDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(buffers), buffers);
}

void Renderer::render() {
    //
    // Clear the backbuffer
    //
    //float ClearColor[4] = { 0.8f, 0.4f, 0.0f, 1.0f }; // RGBA
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // RGBA
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, ClearColor);

    //
    // Clear the depth buffer to 1.0 (max depth)
    //
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	(m_bTessellation ? m_psgTesselatedPhong : m_psgPhong)->use(m_pDeviceContext);
	// Set primitive topology
    m_pDeviceContext->IASetPrimitiveTopology(
		m_bTessellation ? D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	updateTransform();
	updateLighting();
	setConstantBuffers();

	renderScene();

	m_pSwapChain->Present(m_pConfig->vsync ? 1 : 0, 0);

	computeStats();
}

void Renderer::renderScene() {
	for (Renderable* renderable : m_vpRenderables) {
		renderable->render(m_pDeviceContext, this, *m_pCamera);
	}
}

void Renderer::toggleWireframe() {
	// Use wireframe rendering
	if (m_descRasterizerState.FillMode == D3D11_FILL_SOLID) {
		m_descRasterizerState.FillMode = D3D11_FILL_WIREFRAME;
		m_descRasterizerState.CullMode = D3D11_CULL_NONE;
	} else {
		m_descRasterizerState.FillMode = D3D11_FILL_SOLID;
		m_descRasterizerState.CullMode = D3D11_CULL_BACK;
	}
	ID3D11RasterizerState* pRS = nullptr;
	m_pDevice->CreateRasterizerState(&m_descRasterizerState, &pRS);
	m_pDeviceContext->RSSetState(pRS);
	pRS->Release();
}

void Renderer::toggleTessellation() {
	m_bTessellation = !m_bTessellation;
}

void Renderer::setMaterial(const Material& mt) {
	m_cbMaterial.g_mtAmbient = XMColor3(mt.coAmbient);
	m_cbMaterial.g_mtDiffuse = XMColor3(mt.coDiffuse);
	m_cbMaterial.g_mtSpecular = XMColor3(mt.coSpecular);
	m_cbMaterial.g_mtEmissive = XMColor3(mt.coEmissive);
	m_cbMaterial.g_mtShininess = mt.fShininess;
	m_pDeviceContext->UpdateSubresource(m_pMaterialConstantBuffer, 0, NULL, &m_cbMaterial, 0, 0);
}

void Renderer::setWorldMatrix(const XMMATRIX& matWorld) {
	XMStoreFloat4x4(&m_cbTransform.g_matWorld, XMMatrixTranspose(matWorld));
	m_pDeviceContext->UpdateSubresource(m_pTransformConstantBuffer, 0, NULL, &m_cbTransform, 0, 0);
}

void Renderer::usePlaceholderTexture() {
	m_pDeviceContext->PSSetSamplers(0, 1, &m_pPlaceholderSamplerState);
	m_pDeviceContext->PSSetShaderResources(0, 1, &m_pPlaceholderTexture);
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
