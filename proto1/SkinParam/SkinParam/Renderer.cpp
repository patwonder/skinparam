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
#include "DirectXTex.h"
#include "DXUT.h"

using namespace Skin;
using namespace Utils;
using namespace D3DHelper;
using namespace DirectX;

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

const float Renderer::CLIPPING_LIGHT_NEAR = 5.0f;
const float Renderer::CLIPPING_LIGHT_FAR = 20.0f;
const float Renderer::FOV_LIGHT = 50.0f;
const float Renderer::CLIPPING_SCENE_NEAR = 0.1f;
const float Renderer::CLIPPING_SCENE_FAR = 20.0f;
const float Renderer::FOV_SCENE = 30.0f;
const float Renderer::MM_PER_LENGTH = 120.0f;

const float Renderer::SSS_GAUSSIAN_KERNEL_SIGMA[NUM_SSS_GAUSSIANS] = {
	0.0064f, 0.0484f, 0.187f, 0.567f, 1.99f, 7.41f
};

const float Renderer::SSS_GAUSSIAN_WEIGHTS[NUM_SSS_GAUSSIANS][3] = {
	{ 0.233f, 0.455f, 0.649f },
	{ 0.100f, 0.336f, 0.344f },
	{ 0.118f, 0.198f, 0.000f },
	{ 0.113f, 0.007f, 0.007f },
	{ 0.358f, 0.004f, 0.000f },
	{ 0.078f, 0.000f, 0.000f }
};

Renderer::Renderer(HWND hwnd, CRect rectView, Config* pConfig, Camera* pCamera, RenderableManager* pRenderableManager)
	: m_pDevice(nullptr),
	  m_pDeviceContext(nullptr),
	  m_pSwapChain(nullptr),
	  m_pRenderTargetView(nullptr),
	  m_pDepthStencil(nullptr),
	  m_pDepthStencilView(nullptr),
	  m_pPlaceholderSamplerState(nullptr),
	  m_pPlaceholderTexture(nullptr),
	  m_pBumpSamplerState(nullptr),
	  m_pBumpTexture(nullptr),
	  m_pNormalTexture(nullptr),
	  m_pLinearSamplerState(nullptr),
	  m_pShadowMapDepthStencilView(nullptr),
	  m_pShadowMapSamplerState(nullptr),
	  m_pTransformConstantBuffer(nullptr),
	  m_pLightingConstantBuffer(nullptr),
	  m_pMaterialConstantBuffer(nullptr),
	  m_pTessellationConstantBuffer(nullptr),
	  m_pGaussianConstantBuffer(nullptr),
	  m_pCombineConstantBuffer(nullptr),
	  m_pPostProcessConstantBuffer(nullptr),
	  m_pGaussianShadowConstantBuffer(nullptr),
	  m_pSSSConstantBuffer(nullptr),
	  m_psgSSSIrradianceNoTessellation(nullptr),
	  m_psgShadow(nullptr),
	  m_psgTessellatedPhong(nullptr),
	  m_psgTessellatedShadow(nullptr),
	  m_psgSSSIrradiance(nullptr),
	  m_psgSSSGausianVertical(nullptr),
	  m_psgSSSGausianHorizontal(nullptr),
	  m_psgSSSCombine(nullptr),
	  m_psgSSSCombineAA(nullptr),
	  m_psgPostProcessAA(nullptr),
	  m_psgGaussianShadowVertical(nullptr),
	  m_psgGaussianShadowHorizontal(nullptr),
	  m_psgCopy(nullptr),
	  m_psgCopyLinear(nullptr),
	  m_hwnd(hwnd),
	  m_rectView(rectView),
	  m_pConfig(pConfig),
	  m_pCamera(pCamera),
	  m_pRenderableManager(pRenderableManager),
	  m_nFrameCount(0),
	  m_nStartTick(GetTickCount()),
	  m_fps(0.0f),
	  m_bTessellation(true),
	  m_bBump(true),
	  m_bSSS(true),
	  m_bPostProcessAA(true),
	  m_bVSMBlur(true),
	  m_bDump(false),
	  m_rsCurrent(RS_NotRendering)
{
	memset(m_apRTShadowMaps, 0, sizeof(m_apRTShadowMaps));
	memset(m_apSRVShadowMaps, 0, sizeof(m_apSRVShadowMaps));
	memset(m_apRTSSS, 0, sizeof(m_apRTSSS));
	memset(m_apSRVSSS, 0, sizeof(m_apSRVSSS));

	m_vppCOMObjs.push_back((IUnknown**)&m_pDevice);
	m_vppCOMObjs.push_back((IUnknown**)&m_pDeviceContext);
	m_vppCOMObjs.push_back((IUnknown**)&m_pSwapChain);
	m_vppCOMObjs.push_back((IUnknown**)&m_pRenderTargetView);
	m_vppCOMObjs.push_back((IUnknown**)&m_pDepthStencil);
	m_vppCOMObjs.push_back((IUnknown**)&m_pDepthStencilView);
	m_vppCOMObjs.push_back((IUnknown**)&m_pPlaceholderSamplerState);
	m_vppCOMObjs.push_back((IUnknown**)&m_pPlaceholderTexture);
	m_vppCOMObjs.push_back((IUnknown**)&m_pBumpSamplerState);
	m_vppCOMObjs.push_back((IUnknown**)&m_pBumpTexture);
	m_vppCOMObjs.push_back((IUnknown**)&m_pNormalTexture);
	m_vppCOMObjs.push_back((IUnknown**)&m_pLinearSamplerState);
	m_vppCOMObjs.push_back((IUnknown**)&m_pShadowMapDepthStencilView);
	m_vppCOMObjs.push_back((IUnknown**)&m_pShadowMapSamplerState);
	m_vppCOMObjs.push_back((IUnknown**)&m_pTransformConstantBuffer);
	m_vppCOMObjs.push_back((IUnknown**)&m_pLightingConstantBuffer);
	m_vppCOMObjs.push_back((IUnknown**)&m_pMaterialConstantBuffer);
	m_vppCOMObjs.push_back((IUnknown**)&m_pTessellationConstantBuffer);
	m_vppCOMObjs.push_back((IUnknown**)&m_pGaussianConstantBuffer);
	m_vppCOMObjs.push_back((IUnknown**)&m_pCombineConstantBuffer);
	m_vppCOMObjs.push_back((IUnknown**)&m_pPostProcessConstantBuffer);
	m_vppCOMObjs.push_back((IUnknown**)&m_pGaussianShadowConstantBuffer);
	m_vppCOMObjs.push_back((IUnknown**)&m_pSSSConstantBuffer);
	for (UINT i = 0; i < NUM_SHADOW_VIEWS; i++) {
		m_vppCOMObjs.push_back((IUnknown**)m_apRTShadowMaps + i);
		m_vppCOMObjs.push_back((IUnknown**)m_apSRVShadowMaps + i);
	}
	for (UINT i = 0; i < NUM_SSS_VIEWS; i++) {
		m_vppCOMObjs.push_back((IUnknown**)m_apRTSSS + i);
		m_vppCOMObjs.push_back((IUnknown**)m_apSRVSSS + i);
	}

	m_vppShaderGroups.push_back(&m_psgSSSIrradianceNoTessellation);
	m_vppShaderGroups.push_back(&m_psgShadow);
	m_vppShaderGroups.push_back(&m_psgTessellatedPhong);
	m_vppShaderGroups.push_back(&m_psgTessellatedShadow);
	m_vppShaderGroups.push_back(&m_psgSSSIrradiance);
	m_vppShaderGroups.push_back(&m_psgSSSGausianVertical);
	m_vppShaderGroups.push_back(&m_psgSSSGausianHorizontal);
	m_vppShaderGroups.push_back(&m_psgSSSCombine);
	m_vppShaderGroups.push_back(&m_psgSSSCombineAA);
	m_vppShaderGroups.push_back(&m_psgPostProcessAA);
	m_vppShaderGroups.push_back(&m_psgGaussianShadowVertical);
	m_vppShaderGroups.push_back(&m_psgGaussianShadowHorizontal);
	m_vppShaderGroups.push_back(&m_psgCopy);
	m_vppShaderGroups.push_back(&m_psgCopyLinear);

	checkFailure(initDX(), _T("Failed to initialize DirectX 11"));

	initMisc();

	loadShaders();
	initTransform();
	initLighting();
	initMaterial();
	updateProjection();
	initTessellation();
	initShadowMaps();

	initSSS();
	initPostProcessAA();
}

Renderer::~Renderer() {
	removeAllRenderables();
	unloadShaders();

	m_pRenderableManager->onReleasingSwapChain();
	m_pRenderableManager->onDestroyDevice();

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

	DXUTDeviceSettings ds;
	ZeroMemory(&ds, sizeof(ds));
	ds.MinimumFeatureLevel = D3D_FEATURE_LEVEL_11_0;
	ds.ver = DXUT_D3D11_DEVICE;
	ds.d3d11.AdapterOrdinal = 0;
    ds.d3d11.CreateFlags = 0;
    ds.d3d11.DriverType = D3D_DRIVER_TYPE_HARDWARE;
	ds.d3d11.DeviceFeatureLevel = D3D_FEATURE_LEVEL_11_0;
    ds.d3d11.Output = 0;
    ds.d3d11.PresentFlags = 0;
    ds.d3d11.SyncInterval = 1;
    ds.d3d11.AutoCreateDepthStencil = false;
	ds.d3d11.AutoDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	DXGI_SWAP_CHAIN_DESC& sd = ds.d3d11.sd;
    sd.BufferCount = 1;
	sd.BufferDesc.Width = m_rectView.Width();
	sd.BufferDesc.Height = m_rectView.Height();
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

	HRESULT hr;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
		m_driverType = driverTypes[driverTypeIndex];
		ds.d3d11.DriverType = m_driverType;
		hr = DXUTCreateDeviceFromSettings(&ds, false, true);
        if (SUCCEEDED(hr))
            break;
    }
    if (FAILED(hr))
        return hr;

	m_pSwapChain = DXUTGetDXGISwapChain();
	m_pDevice = DXUTGetD3D11Device();
	m_featureLevel = DXUTGetD3D11DeviceFeatureLevel();
	m_pDeviceContext = DXUTGetD3D11DeviceContext();
	m_pSwapChain->AddRef();
	m_pDevice->AddRef();
	m_pDeviceContext->AddRef();

	m_pRenderableManager->onCreateDevice(m_pDevice, m_pDeviceContext, m_pSwapChain);
	
	const DXGI_SURFACE_DESC* pDesc = DXUTGetDXGIBackBufferSurfaceDesc();
	m_rectView = CRect(0, 0, pDesc->Width, pDesc->Height);
	m_pRenderableManager->onResizedSwapChain(m_pDevice, pDesc);

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
    D3D11_VIEWPORT& vp = m_vpScreen;
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

    return S_OK;
}

void Renderer::initMisc() {
	// Creates the placeholder texture for non-textured objects
	checkFailure(createSamplerState(m_pDevice, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, &m_pPlaceholderSamplerState),
		_T("Failed to create placeholder sampler state"));

	checkFailure(createTextureResourceView(m_pDevice, 1, 1, DXGI_FORMAT_R32_FLOAT, &m_pPlaceholderTexture),
		_T("Failed to create placeholder texture"));

	{
		float initData[] = { 1.0f };
		ID3D11Resource* pResource = nullptr;
		m_pPlaceholderTexture->GetResource(&pResource);
		m_pDeviceContext->UpdateSubresource(pResource, 0, nullptr, initData, 4, 0);
		pResource->Release();
	}

	// Creates the placeholder bump texture for non-bumpmapped objects
	checkFailure(createSamplerState(m_pDevice, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, &m_pBumpSamplerState),
		_T("Failed to create placeholder bump sampler state"));

	checkFailure(createTextureResourceView(m_pDevice, 1, 1, DXGI_FORMAT_R32_FLOAT, &m_pBumpTexture),
		_T("Failed to create placeholder bump map"));

	{
		float initData[] = { 0.5f };
		ID3D11Resource* pResource = nullptr;
		m_pBumpTexture->GetResource(&pResource);
		m_pDeviceContext->UpdateSubresource(pResource, 0, nullptr, initData, 4, 0);
		pResource->Release();
	}

	// Creates the placeholder normal texture for non-normalmapped objects
	checkFailure(createTextureResourceView(m_pDevice, 1, 1, DXGI_FORMAT_R32G32_FLOAT, &m_pNormalTexture),
		_T("Failed to create placeholder normal map"));
	{
		float initData[] = { 0.0f, 0.0f };
		ID3D11Resource* pResource = nullptr;
		m_pNormalTexture->GetResource(&pResource);
		m_pDeviceContext->UpdateSubresource(pResource, 0, nullptr, initData, 8, 0);
		pResource->Release();
	}

	// Creates a linear sampler state for shadow maps
	checkFailure(createSamplerState(m_pDevice, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, &m_pLinearSamplerState),
		_T("Failed to create linear sampler state"));
}

void Renderer::addRenderable(Renderable* renderable) {
	renderable->init(m_pDevice, this);
	m_vpRenderables.push_back(renderable);
}

void Renderer::removeRenderable(Renderable* renderable) {
	for (size_t i = 0; i < m_vpRenderables.size(); i++) {
		if (m_vpRenderables[i] == renderable) {
			m_vpRenderables.erase(m_vpRenderables.begin() + i);
			renderable->cleanup(this);
			break;
		}
	}
}

void Renderer::removeAllRenderables() {
	for (Renderable* pRenderable : m_vpRenderables) {
		pRenderable->cleanup(this);
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
	// World-space shaders
	D3D11_INPUT_ELEMENT_DESC phong_layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	m_psgTessellatedPhong = new ShaderGroup(m_pDevice, _T("TessellatedPhong.fx"), phong_layout, ARRAYSIZE(phong_layout),
		"VS", "HS", "DS", "PS");

	m_psgShadow = new ShaderGroup(m_pDevice, _T("TessellatedShadow.fx"), phong_layout, ARRAYSIZE(phong_layout),
		"VS_NoTessellation", nullptr, nullptr, "PS");

	m_psgTessellatedShadow = new ShaderGroup(m_pDevice, _T("TessellatedShadow.fx"), phong_layout, ARRAYSIZE(phong_layout),
		"VS", "HS", "DS", "PS");

	// SSS
	m_psgSSSIrradiance = new ShaderGroup(m_pDevice, _T("TessellatedPhong.fx"), phong_layout, ARRAYSIZE(phong_layout),
		"VS", "HS", "DS_Irradiance", "PS_Irradiance");

	m_psgSSSIrradianceNoTessellation = new ShaderGroup(m_pDevice, _T("TessellatedPhong.fx"), phong_layout, ARRAYSIZE(phong_layout),
		"VS_Irradiance_NoTessellation", nullptr, nullptr, "PS_Irradiance");

	// Screen-space shaders
	D3D11_INPUT_ELEMENT_DESC empty_layout[] = { { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 } };
	// Copy shader
	m_psgCopy = new ShaderGroup(m_pDevice, _T("Copy.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS");
	m_psgCopyLinear = new ShaderGroup(m_pDevice, _T("Copy.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS_Linear");

	// Subsurface Scattering
	m_psgSSSGausianVertical = new ShaderGroup(m_pDevice, _T("Gaussian.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS_Vertical");
	m_psgSSSGausianHorizontal = new ShaderGroup(m_pDevice, _T("Gaussian.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS_Horizontal");
	m_psgSSSCombine = new ShaderGroup(m_pDevice, _T("Combine.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS");
	m_psgSSSCombineAA = new ShaderGroup(m_pDevice, _T("Combine.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS_AA");

	// Post-process Anti-aliasing
	m_psgPostProcessAA = new ShaderGroup(m_pDevice, _T("PostProcessAA.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS");

	// Shadow map blurring
	m_psgGaussianShadowVertical = new ShaderGroup(m_pDevice, _T("GaussianShadow.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS_Vertical");
	m_psgGaussianShadowHorizontal = new ShaderGroup(m_pDevice, _T("GaussianShadow.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS_Horizontal");
}

void Renderer::unloadShaders() {
	for (ShaderGroup** ppsg : m_vppShaderGroups) {
		if (*ppsg)
			delete *ppsg;
		*ppsg = nullptr;
	}

	m_vppShaderGroups.clear();
}

void Renderer::initTransform() {
	XMStoreFloat4x4(&m_cbTransform.g_matWorld, XMMatrixIdentity());
	XMStoreFloat4x4(&m_cbTransform.g_matView, XMMatrixIdentity());
	XMStoreFloat4x4(&m_cbTransform.g_matViewProj, XMMatrixIdentity());
	m_cbTransform.g_posEye = XMPos(Vector::ZERO);

	m_cbTransform.g_fAspectRatio = (float)m_rectView.Width() / m_rectView.Height();

	XMFLOAT4 vFrustrumPlanes[4] = {
		XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)
	};
	memcpy(m_cbTransform.g_vFrustumPlaneEquation, vFrustrumPlanes, sizeof(m_cbTransform.g_vFrustumPlaneEquation));

	XMStoreFloat4x4(&m_cbTransform.g_matViewProjCamera, XMMatrixIdentity());
	for (UINT i = 0; i < NUM_LIGHTS; i++) {
		XMStoreFloat4x4(m_cbTransform.g_matViewLights + i, XMMatrixIdentity());
		XMStoreFloat4x4(m_cbTransform.g_matViewProjLights + i, XMMatrixIdentity());
	}

	checkFailure(createConstantBuffer(m_pDevice, &m_cbTransform, &m_pTransformConstantBuffer),
		_T("Failed to create transform constant buffer"));
}

XMMATRIX Renderer::getViewProjMatrix(const Camera& camera, const XMMATRIX& matProjection, XMMATRIX& matView) {
	Vector vEye, vLookAt, vUp;
	camera.look(vEye, vLookAt, vUp);

	matView = XMMatrixLookAtLH(XMVec(vEye), XMVec(vLookAt), XMVec(vUp));
	return XMMatrixMultiply(matView, matProjection);
}

Camera Renderer::getLightCamera(const Light& light) {
	return Camera(light.vecPosition, Vector(0, 0, -0.5), Vector(0, 0, 1));
}

void Renderer::updateTransform() {
	updateTransform(*m_pCamera, XMLoadFloat4x4(&m_matProjection), (float)m_rectView.Width() / m_rectView.Height());
}

void Renderer::updateTransformForLight(const Light& light) {
	updateTransform(getLightCamera(light), XMLoadFloat4x4(&m_matLightProjection), (float)SM_SIZE / SM_SIZE);
}

void Renderer::updateTransform(const Camera& camera, const XMMATRIX& matProjection, float fAspectRatio) {
	XMMATRIX matView;
	XMMATRIX matViewProj = getViewProjMatrix(camera, matProjection, matView);
	XMStoreFloat4x4(&m_cbTransform.g_matView, XMMatrixTranspose(matView));
	XMStoreFloat4x4(&m_cbTransform.g_matViewProj, XMMatrixTranspose(matViewProj));

	m_cbTransform.g_posEye = XMPos(camera.getVecEye());

	// update frustum
	XMFLOAT4 vFrustumPlanes[6];
	m_frustum.CalculateFrustum(matViewProj, vFrustumPlanes);
	memcpy(m_cbTransform.g_vFrustumPlaneEquation, vFrustumPlanes, sizeof(m_cbTransform.g_vFrustumPlaneEquation));

	m_cbTransform.g_fAspectRatio = fAspectRatio;

	// param-independent data
	XMStoreFloat4x4(&m_cbTransform.g_matViewProjCamera, 
		XMMatrixTranspose(getViewProjMatrix(*m_pCamera, XMLoadFloat4x4(&m_matProjection), matView)));

	for (UINT i = 0; i < NUM_LIGHTS && i < m_vpLights.size(); i++) {
		const Light* l = m_vpLights[i];
		XMMATRIX matViewProj = getViewProjMatrix(getLightCamera(*l),
			XMLoadFloat4x4(&m_matLightProjection), matView);
		XMStoreFloat4x4(m_cbTransform.g_matViewLights + i, XMMatrixTranspose(matView));
		XMStoreFloat4x4(m_cbTransform.g_matViewProjLights + i, XMMatrixTranspose(matViewProj));
	}

	m_pDeviceContext->UpdateSubresource(m_pTransformConstantBuffer, 0, NULL, &m_cbTransform, 0, 0);
}

void Renderer::setGlobalAmbient(const Color& coAmbient) {
	m_cbLighting.g_ambient = XMColor3(coAmbient);
}

void Renderer::initLighting() {
	m_cbLighting.g_ambient = XMColor3(Color::Black);
	for (UINT i = 0; i < NUM_LIGHTS; i++) {
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
	for (UINT i = 0; i < NUM_LIGHTS && i < m_vpLights.size(); i++) {
		RLight& rl = m_cbLighting.g_lights[i];
		const Light* l = m_vpLights[i];
		rl.ambient = XMColor3(l->coAmbient);
		rl.diffuse = XMColor3(l->coDiffuse);
		rl.specular = XMColor3(l->coSpecular);
		rl.position = XMPos(l->vecPosition);
		rl.attenuation = XMFLOAT3(l->fAtten0, l->fAtten1, l->fAtten2);
	}
	for (UINT i = m_vpLights.size(); i < NUM_LIGHTS; i++) {
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
	m_cbMaterial.g_mtBumpMultiplier = 0.0f;

	checkFailure(createConstantBuffer(m_pDevice, &m_cbMaterial, &m_pMaterialConstantBuffer),
		_T("Failed to create material constant buffer"));
}

void Renderer::updateProjection() {
	XMStoreFloat4x4(&m_matProjection,
		XMMatrixPerspectiveFovLH((float)(Math::PI * FOV_SCENE / 180.0), (float)m_rectView.Width() / m_rectView.Height(), CLIPPING_SCENE_NEAR, CLIPPING_SCENE_FAR));
	XMStoreFloat4x4(&m_matLightProjection,
		XMMatrixPerspectiveFovLH((float)(Math::PI * FOV_LIGHT / 180.0), 1.0f, CLIPPING_LIGHT_NEAR, CLIPPING_LIGHT_FAR));
}

void Renderer::initTessellation() {
	// Edge, inside, minimum tessellation factor and (half screen height/desired triangle size)
	m_cbTessellation.g_vTesselationFactor = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f);
	checkFailure(createConstantBuffer(m_pDevice, &m_cbTessellation, &m_pTessellationConstantBuffer),
		_T("Failed to create tessellation constant buffer"));
}

void Renderer::updateTessellation() {
}

void Renderer::initShadowMaps() {
	for (UINT i = 0; i < NUM_SHADOW_VIEWS; i++) {
		ID3D11Texture2D* pTexture2D = nullptr;
		// VSM: 2 channels & turn mip-mapping on
		DXGI_FORMAT format = DXGI_FORMAT_R32G32_FLOAT;
		// Create the temporary WITHOUT mip-mapping on
		checkFailure(createTexture2DEx(m_pDevice, SM_SIZE, SM_SIZE, format, i == IDX_SHADOW_TEMPORARY ? true : true, 1, 0,
			&pTexture2D, (D3D11_BIND_FLAG)(D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET)),
			_T("Failed to create shadow map texture"));

		checkFailure(createShaderResourceView(m_pDevice, pTexture2D, format, m_apSRVShadowMaps + i),
			_T("Failed to create SRV for shadow map"));

		checkFailure(createRenderTargetView(m_pDevice, pTexture2D, format, m_apRTShadowMaps + i),
			_T("Failed to create RTV for shadow map"));

		pTexture2D->Release();
	}

	// Create depth stencil view for shadow maps
	ID3D11Texture2D* pTexture2D = nullptr;
	checkFailure(createTexture2D(m_pDevice, SM_SIZE, SM_SIZE, DXGI_FORMAT_D24_UNORM_S8_UINT,
		&pTexture2D, D3D11_BIND_DEPTH_STENCIL),
		_T("Failed to create depth stencil texture for shadow maps"));

    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
	checkFailure(m_pDevice->CreateDepthStencilView(pTexture2D, &descDSV, &m_pShadowMapDepthStencilView),
		_T("Failed to create depth stencil view for shadow maps"));

	pTexture2D->Release();

	// Create shadow map sampler state	
	checkFailure(createSamplerComparisonState(m_pDevice, D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
		D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_COMPARISON_LESS, &m_pShadowMapSamplerState),
		_T("Failed to create shadow map sampler comparison state"));

	// Initialize shadow map view port
	D3D11_VIEWPORT& vp = m_vpShadowMap;
	vp.Width = SM_SIZE;
	vp.Height = SM_SIZE;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;

	// Initialize constant buffer for shadow map blurring
	m_cbGaussainShadow.rcpScreenSize = XMFLOAT2(1.0f / SM_SIZE, 1.0f / SM_SIZE);
	checkFailure(createConstantBuffer(m_pDevice, &m_cbGaussainShadow, &m_pGaussianShadowConstantBuffer),
		_T("Failed to create gaussian shadow constant buffer"));
}

void Renderer::bindShadowMaps() {
	// use trilinear sampler for shadow maps
	m_pDeviceContext->PSSetSamplers(SLOT_SHADOWMAP, 1, &m_pLinearSamplerState);
	m_pDeviceContext->PSSetShaderResources(SLOT_SHADOWMAP, NUM_LIGHTS, m_apSRVShadowMaps);
}

void Renderer::unbindShadowMaps() {
	ID3D11ShaderResourceView* pNullSRVs[NUM_LIGHTS] = { nullptr };
	m_pDeviceContext->PSSetShaderResources(SLOT_SHADOWMAP, NUM_LIGHTS, pNullSRVs);
}

void Renderer::initSSS() {
	for (UINT i = 0; i < NUM_SSS_VIEWS; i++) {
		ID3D11Texture2D* pTexture2D = nullptr;
		DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		checkFailure(createTexture2D(m_pDevice, m_rectView.Width(), m_rectView.Height(), format,
			&pTexture2D, (D3D11_BIND_FLAG)(D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET)),
			_T("Failed to create SSS texture"));

		checkFailure(createShaderResourceView(m_pDevice, pTexture2D, format, m_apSRVSSS + i),
			_T("Failed to create SRV for SSS"));

		checkFailure(createRenderTargetView(m_pDevice, pTexture2D, format, m_apRTSSS + i),
			_T("Failed to create RTV for SSS"));

		pTexture2D->Release();
	}
	// no need to create depth stencil view, just re-use the screen one

	// initialize related constant buffers
	m_cbGaussian.g_blurWidth = 0.0f;
	m_cbGaussian.g_screenWidth = (float)m_rectView.Width();
	m_cbGaussian.g_screenHeight = (float)m_rectView.Height();
	m_cbGaussian.g_invAspectRatio = (float)m_rectView.Height() / m_rectView.Width();
	checkFailure(createConstantBuffer(m_pDevice, &m_cbGaussian, &m_pGaussianConstantBuffer),
		_T("Failed to create gaussian constant buffer"));

	for (UINT i = 0; i < NUM_SSS_GAUSSIANS; i++) {
		m_cbCombine.g_weights[i].value = XMFLOAT3(SSS_GAUSSIAN_WEIGHTS[i]);
	}
	checkFailure(createConstantBuffer(m_pDevice, &m_cbCombine, &m_pCombineConstantBuffer),
		_T("Failed to create combine constant buffer"));

	m_cbSSS.g_sss_intensity = 0.0f;
	m_cbSSS.g_sss_strength = 1.0f;
	checkFailure(createConstantBuffer(m_pDevice, &m_cbSSS, &m_pSSSConstantBuffer),
		_T("Failed to create SSS constant buffer"));
}

void Renderer::initPostProcessAA() {
	m_cbPostProcess.rcpFrame = XMFLOAT2(1.0f / m_rectView.Width(), 1.0f / m_rectView.Height());
	m_cbPostProcess.rcpFrameOpt = XMFLOAT4(2.0f / m_rectView.Width(), 2.0f / m_rectView.Height(), 0.5f / m_rectView.Width(), 0.5f / m_rectView.Height());

	checkFailure(createConstantBuffer(m_pDevice, &m_cbPostProcess, &m_pPostProcessConstantBuffer),
		_T("Failed to create post-process constant buffer"));
}

void Renderer::setConstantBuffers() {
	ID3D11Buffer* buffers[] = {
		m_pTransformConstantBuffer,
		m_pLightingConstantBuffer,
		m_pMaterialConstantBuffer,
		m_pTessellationConstantBuffer,
		m_pSSSConstantBuffer,
	};

	m_pDeviceContext->VSSetConstantBuffers(0, ARRAYSIZE(buffers), buffers);
	m_pDeviceContext->HSSetConstantBuffers(0, ARRAYSIZE(buffers), buffers);
	m_pDeviceContext->DSSetConstantBuffers(0, ARRAYSIZE(buffers), buffers);
	m_pDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(buffers), buffers);
}

void Renderer::renderIrradianceMap(ID3D11RenderTargetView* pRTIrradiance, ID3D11RenderTargetView* pRTAlbedo,
								   ID3D11RenderTargetView* pRTDiffuseStencil, ID3D11RenderTargetView* pRTSpecular,
								   bool* opbNeedBlur) {

	setConstantBuffers();

	m_pDeviceContext->IASetPrimitiveTopology(
		m_bTessellation ? D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Render the irradiance map
	(m_bTessellation ? m_psgSSSIrradiance : m_psgSSSIrradianceNoTessellation)->use(m_pDeviceContext);

	ID3D11RenderTargetView* apSSSRenderTargets[] = {
		pRTIrradiance,
		pRTAlbedo,
		pRTDiffuseStencil,
		pRTSpecular
	};
	m_pDeviceContext->OMSetRenderTargets(ARRAYSIZE(apSSSRenderTargets), apSSSRenderTargets, m_pDepthStencilView);
	// Clear render target view & depth stencil
	float ClearIrradiance[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // RGBA
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // RGBA
	float ClearDiffuseStencil[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
	m_pDeviceContext->ClearRenderTargetView(pRTIrradiance, ClearIrradiance);
	m_pDeviceContext->ClearRenderTargetView(pRTAlbedo, ClearColor);
	m_pDeviceContext->ClearRenderTargetView(pRTDiffuseStencil, ClearDiffuseStencil);
	m_pDeviceContext->ClearRenderTargetView(pRTSpecular, ClearColor);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	updateTransform();
	bindShadowMaps();
	renderScene(opbNeedBlur);
	unbindShadowMaps();

	if (m_bDump) {
		dumpRenderTargetToFile(pRTIrradiance, _T("Irradiance"));
		dumpRenderTargetToFile(pRTAlbedo, _T("Albedo"));
		dumpRenderTargetToFile(pRTDiffuseStencil, _T("DiffuseStencil"));
		dumpRenderTargetToFile(pRTSpecular, _T("Specular"));
	}
}

void Renderer::unbindInputBuffers() {
	// unbind all vertex and index buffers
	UINT zero = 0;
	ID3D11Buffer* nullBuffer = nullptr;
	m_pDeviceContext->IASetVertexBuffers(0, 1, &nullBuffer, &zero, &zero);
	m_pDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
}

void Renderer::bindGaussianConstantBuffer() {
	m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pGaussianConstantBuffer);
}

void Renderer::bindCombineConstantBuffer() {
	m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pCombineConstantBuffer);
}

void Renderer::setGaussianKernelSize(float size) {
	m_cbGaussian.g_blurWidth = size;
	m_pDeviceContext->UpdateSubresource(m_pGaussianConstantBuffer, 0, nullptr, &m_cbGaussian, 0, 0);
}

// Compute the min depth of objects from camera
float Renderer::getMinDepthForScene() const {
	float depth = FLT_MAX;
	float MM_THRESHOLD = 0.1f;
	for (Renderable* r : m_vpRenderables) {
		if (!r->inScene() || !r->supportsSSS()) continue;
		FVector vCenter; float radius;
		r->getBoundingSphere(vCenter, radius);
		// ignore things that are less than 0.1mm in size
		if (radius < MM_THRESHOLD / MM_PER_LENGTH)
			continue;
		// ignore things out of the viewing frustum
		if (!m_frustum.SphereIntersectsFrustum(XMVectorSet(vCenter.x, vCenter.y, vCenter.z, 1.0f), radius))
			continue;

		float shadowLength = (float)((Vector(vCenter.x, vCenter.y, vCenter.z) - m_pCamera->getVecEye())
			* (m_pCamera->getVecLookAt() - m_pCamera->getVecEye()).normalize());
		float d = max(CLIPPING_SCENE_NEAR, shadowLength - radius);
		if (d < depth)
			depth = d;
	}
	return depth;
}

// estimates maximum gaussian kernel size in pixels
float Renderer::estimateGaussianKernelSize(float standardDeviation, float minDepth) {
	// take the distance to the camera as the depth estimation
	if (minDepth > CLIPPING_SCENE_FAR) return 0.0f;

	static const float SIZE_ALPHA = (float)(1.0 / (2.0 * tan((FOV_SCENE / 180.0 * Math::PI) / 2.0) * MM_PER_LENGTH));
	float kernelSizeInPixels = standardDeviation * SIZE_ALPHA / minDepth * (float)m_rectView.Height() * m_cbSSS.g_sss_strength;
	return kernelSizeInPixels;
}

bool Renderer::selectShaderGroupsForKernelSize(float kernelSizeInPixels, ShaderGroup** ppsgVertical, ShaderGroup** ppsgHorizontal) {
	// Filt  SD  Bound    B7   B5   B3   BMP  BM    SDu  SDl   SDM
	// 7-tap 1.0 x * 3    3.0  2.4  1.5  ...  2.18  1.0  0.67  0.727
	// 5-tap 0.8 x * 2.5  2.5  2.0  1.25 1.82 1.11  0.8  0.4   0.444
	// 3-tap 0.5 x * 2.0  2.0  1.6  1.0  .889 0.20  0.5  0.0   0.10
	// SDM(Mean Standard Deviation) taken to be the harmonic mean of SDl and next SDu

	// 7-tap to 5-tap limit
	static const float SEVEN_TO_FIVE = 0.727f;
	// 5-tap to 3-tap limit
	static const float FIVE_TO_THREE = 0.444f;
	// 3-tap to copy limit
	static const float THREE_TO_ONE = 0.1f;

	if (kernelSizeInPixels > SEVEN_TO_FIVE) {
		// standard 7 tap filter
		*ppsgVertical = m_psgSSSGausianVertical;
		*ppsgHorizontal = m_psgSSSGausianHorizontal;
	} else if (kernelSizeInPixels > FIVE_TO_THREE) {
		*ppsgVertical = m_psgSSSGausianVertical;
		*ppsgHorizontal = m_psgSSSGausianHorizontal;
	} else if (kernelSizeInPixels > THREE_TO_ONE) {
		*ppsgVertical = m_psgSSSGausianVertical;
		*ppsgHorizontal = m_psgSSSGausianHorizontal;
	} else {
		// just copy pixels
		*ppsgVertical = m_psgCopy;
		*ppsgHorizontal = m_psgCopy;
		return true;
	}
	return false;
}

void Renderer::doGaussianBlurs() {
	bindGaussianConstantBuffer();
	
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->PSSetSamplers(0, 1, &m_pLinearSamplerState);
	m_pDeviceContext->PSSetSamplers(1, 1, &m_pBumpSamplerState);

	ID3D11RenderTargetView* pRTTemporary = m_apRTSSS[IDX_SSS_TEMPORARY];
	ID3D11ShaderResourceView* pSRVTemporary = m_apSRVSSS[IDX_SSS_TEMPORARY];
	
	// previous rendererd shader resource view
	ID3D11ShaderResourceView* pSRVPrevious = m_apSRVSSS[IDX_SSS_IRRADIANCE];

	// shader resources
	ID3D11ShaderResourceView* apSRVs[] = {
		nullptr,
		m_apSRVSSS[IDX_SSS_DIFFUSE_STENCIL]
	};
	const UINT numViews = ARRAYSIZE(apSRVs);

	// null shader resources for unbinding
	ID3D11ShaderResourceView* apNullSRVs[numViews] = { nullptr };

	float minDepth = getMinDepthForScene();
	float prevVariance = 0;
	for (UINT i = 0; i < NUM_SSS_GAUSSIANS; i++) {
		float variance = SSS_GAUSSIAN_KERNEL_SIGMA[i];
		float size = sqrt(variance - prevVariance);
		prevVariance = variance;

		setGaussianKernelSize(size);
		float kernelSizeInPixels = estimateGaussianKernelSize(size, minDepth);
		ShaderGroup* psgGaussianVertical;
		ShaderGroup* psgGaussianHorizontal;
		bool bCopy = selectShaderGroupsForKernelSize(kernelSizeInPixels, &psgGaussianVertical, &psgGaussianHorizontal);

		if (bCopy) {
			// previous ->(copy)-> next
			psgGaussianVertical->use(m_pDeviceContext);
			m_pDeviceContext->OMSetRenderTargets(1, &m_apRTSSS[IDX_SSS_GAUSSIANS_START + i], nullptr);
			apSRVs[0] = pSRVPrevious;
			m_pDeviceContext->PSSetShaderResources(0, numViews, apSRVs);

			m_pDeviceContext->Draw(6, 0);
		} else {
			// do separate gaussian passes
			// previous ->(vertical blur)-> temporary
			psgGaussianVertical->use(m_pDeviceContext);
			m_pDeviceContext->OMSetRenderTargets(1, &pRTTemporary, nullptr);
			apSRVs[0] = pSRVPrevious;
			m_pDeviceContext->PSSetShaderResources(0, numViews, apSRVs);

			m_pDeviceContext->Draw(6, 0);

			// temporary ->(horizontal blur)-> next
			psgGaussianHorizontal->use(m_pDeviceContext);
			m_pDeviceContext->OMSetRenderTargets(1, &m_apRTSSS[IDX_SSS_GAUSSIANS_START + i], nullptr);
			apSRVs[0] = pSRVTemporary;
			m_pDeviceContext->PSSetShaderResources(0, numViews, apSRVs);

			m_pDeviceContext->Draw(6, 0);
		}

		// must unbind shader resource
		m_pDeviceContext->PSSetShaderResources(0, numViews, apNullSRVs);

		pSRVPrevious = m_apSRVSSS[IDX_SSS_GAUSSIANS_START + i];

		if (m_bDump) {
			TStringStream tss;
			tss << _T("Gaussian_") << i + 1;
			dumpShaderResourceViewToFile(pSRVPrevious, tss.str());
		}
	}
}

void Renderer::doCombineShading(bool bNeedBlur) {
	bindCombineConstantBuffer();

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->PSSetSamplers(0, 1, &m_pLinearSamplerState);
	m_pDeviceContext->PSSetSamplers(1, 1, &m_pBumpSamplerState);

	(m_bPostProcessAA ? m_psgSSSCombineAA : m_psgSSSCombine)->use(m_pDeviceContext);

	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // RGBA

	// bind the final render target (before AA)
	ID3D11RenderTargetView* pRenderTargetView = (m_bPostProcessAA ? m_apRTSSS[IDX_SSS_TEMPORARY] : m_pRenderTargetView);
	m_pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);

	// Set up shader resources
	const UINT numViews = 2 + NUM_SSS_GAUSSIANS;
	ID3D11ShaderResourceView* apSSSSRVs[numViews] = {
		m_apSRVSSS[IDX_SSS_ALBEDO],
		m_apSRVSSS[IDX_SSS_SPECULAR],
	};
	if (bNeedBlur)
		memcpy(apSSSSRVs + 2, m_apSRVSSS + IDX_SSS_GAUSSIANS_START, sizeof(*m_apSRVSSS) * NUM_SSS_GAUSSIANS);
	else
		for (UINT i = 0; i < NUM_SSS_GAUSSIANS; i++) {
			apSSSSRVs[2 + i] = m_apSRVSSS[IDX_SSS_IRRADIANCE];
		}
	m_pDeviceContext->PSSetShaderResources(0, numViews, apSSSSRVs);

	// Putting it all together...
	m_pDeviceContext->Draw(6, 0);

	// Unbind all shader resources
	ID3D11ShaderResourceView* apNullSRVs[numViews] = { nullptr };
	m_pDeviceContext->PSSetShaderResources(0, numViews, apNullSRVs);

	if (m_bDump && !m_bPostProcessAA) {
		dumpRenderTargetToFile(pRenderTargetView, _T("Final"));
	}
}

void Renderer::bindPostProcessConstantBuffer() {
	m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pPostProcessConstantBuffer);
}

void Renderer::doPostProcessAA() {
	bindPostProcessConstantBuffer();

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->PSSetSamplers(0, 1, &m_pLinearSamplerState);
	m_pDeviceContext->PSSetSamplers(1, 1, &m_pBumpSamplerState);

	m_psgPostProcessAA->use(m_pDeviceContext);

	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // RGBA

	// bind the (hopefully) final render target
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, nullptr);

	// Set up shader resources, the frame is now in the SSS temporary render target
	m_pDeviceContext->PSSetShaderResources(0, 1, &m_apSRVSSS[IDX_SSS_TEMPORARY]);

	// do it~
	m_pDeviceContext->Draw(6, 0);

	// unbind all shader resources
	ID3D11ShaderResourceView* pNullSRV = nullptr;
	m_pDeviceContext->PSSetShaderResources(0, 1, &pNullSRV);

	if (m_bDump) {
		dumpRenderTargetToFile(m_pRenderTargetView, _T("Final"));
	}
}

void Renderer::blurShadowMaps() {
	m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pGaussianShadowConstantBuffer);

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->PSSetSamplers(0, 1, &m_pLinearSamplerState);
	m_pDeviceContext->PSSetSamplers(1, 1, &m_pBumpSamplerState);

	ID3D11RenderTargetView* pRTTemporary = m_apRTShadowMaps[IDX_SHADOW_TEMPORARY];
	ID3D11ShaderResourceView* pSRVTemporary = m_apSRVShadowMaps[IDX_SHADOW_TEMPORARY];

	for (UINT i = 0; i < NUM_LIGHTS && i < m_vpLights.size(); i++) {
		// shadow map ->(vertical blur)-> temporary
		m_psgGaussianShadowVertical->use(m_pDeviceContext);
		m_pDeviceContext->OMSetRenderTargets(1, &pRTTemporary, nullptr);
		m_pDeviceContext->PSSetShaderResources(0, 1, m_apSRVShadowMaps + i);

		m_pDeviceContext->Draw(6, 0);

		// unbind shader resources
		ID3D11ShaderResourceView* pNullSRV = nullptr;
		m_pDeviceContext->PSSetShaderResources(0, 1, &pNullSRV);

		// temporary ->(horizontal blur)-> shadow map
		m_psgGaussianShadowHorizontal->use(m_pDeviceContext);
		m_pDeviceContext->OMSetRenderTargets(1, m_apRTShadowMaps + i, nullptr);
		m_pDeviceContext->PSSetShaderResources(0, 1, &pSRVTemporary);

		m_pDeviceContext->Draw(6, 0);

		// unbind shader resources
		m_pDeviceContext->PSSetShaderResources(0, 1, &pNullSRV);
	}
}

void Renderer::render() {
	updateLighting();
	updateTessellation();
	setConstantBuffers();

	if (m_bDump)
		m_nDumpCount = 0;

	bool bToggle = (m_descRasterizerState.FillMode == D3D11_FILL_WIREFRAME);
	bool bNeedBlur = false;
	// Render shadow maps
	m_rsCurrent = RS_ShadowMap;
    m_pDeviceContext->IASetPrimitiveTopology(
		m_bTessellation ? D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->RSSetViewports(1, &m_vpShadowMap);
	(m_bTessellation ? m_psgTessellatedShadow : m_psgShadow)->use(m_pDeviceContext);
	if (bToggle) toggleWireframe(); { // Renders solid
		for (UINT i = 0; i < NUM_LIGHTS && i < m_vpLights.size(); i++) {
			m_pDeviceContext->OMSetRenderTargets(1, m_apRTShadowMaps + i, m_pShadowMapDepthStencilView);
			// Clear render target view & depth stencil
			float ClearShadow[4] = { 30.0f, 900.0f, 0.0f, 0.0f }; // RGBA
			m_pDeviceContext->ClearRenderTargetView(m_apRTShadowMaps[i], ClearShadow);
			m_pDeviceContext->ClearDepthStencilView(m_pShadowMapDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

			const Light& l = *m_vpLights[i];
			updateTransformForLight(l);
			renderScene();
		}

		unbindInputBuffers();
		if (m_bVSMBlur)
			blurShadowMaps();

		for (UINT i = 0; i < NUM_LIGHTS && i < m_vpLights.size(); i++) {
			// Generate mip-maps for VSM
			//m_pDeviceContext->GenerateMips(m_apSRVShadowMaps[i]);
			if (m_bDump) {
				TStringStream tss;
				tss << _T("ShadowMap_") << i + 1;
				dumpShaderResourceViewToFile(m_apSRVShadowMaps[i], tss.str());
			}
		}
	} if (bToggle) toggleWireframe();

	m_pDeviceContext->RSSetViewports(1, &m_vpScreen);

	m_rsCurrent = RS_Irradiance;
	renderIrradianceMap(m_apRTSSS[IDX_SSS_IRRADIANCE], m_apRTSSS[IDX_SSS_ALBEDO], 
						m_apRTSSS[IDX_SSS_DIFFUSE_STENCIL], m_apRTSSS[IDX_SSS_SPECULAR],
						&bNeedBlur);

	if (bToggle) toggleWireframe(); {
		if (bNeedBlur) {
			m_rsCurrent = RS_Gaussian;
			doGaussianBlurs();
		}
		m_rsCurrent = RS_Combine;
		doCombineShading(bNeedBlur);

		if (m_bPostProcessAA) {
			m_rsCurrent = RS_PostProcessAA;
			doPostProcessAA();
		}

		m_rsCurrent = RS_UI;
		renderRest();
	} if (bToggle) toggleWireframe();

	m_pSwapChain->Present(m_pConfig->vsync ? 1 : 0, 0);

	m_rsCurrent = RS_NotRendering;

	computeStats();
	m_bDump = false;
}

void Renderer::renderScene(bool* opbNeedBlur) {
	bool bNeedBlur = false;
	for (Renderable* renderable : m_vpRenderables) {
		if (renderable->inScene()) {
			// do frustum culling
			FVector vCenter; float radius;
			renderable->getBoundingSphere(vCenter, radius);
			if (!m_frustum.SphereIntersectsFrustum(XMVectorSet(vCenter.x, vCenter.y, vCenter.z, 1.0f), radius))
				continue;

			// set sss intensity
			m_cbSSS.g_sss_intensity = (m_bSSS && renderable->supportsSSS()) ? 1.0f : 0.0f;
			bNeedBlur |= (m_bSSS && renderable->supportsSSS() && m_cbSSS.g_sss_strength >= 0.02f);
			m_pDeviceContext->UpdateSubresource(m_pSSSConstantBuffer, 0, nullptr, &m_cbSSS, 0, 0);

			renderable->render(m_pDeviceContext, this, *m_pCamera);
		}
	}
	if (opbNeedBlur)
		*opbNeedBlur = bNeedBlur;
}

void Renderer::renderRest() {
	for (Renderable* renderable : m_vpRenderables) {
		if (!renderable->inScene()) {
			renderable->render(m_pDeviceContext, this, *m_pCamera);
		}
	}
}

bool Renderer::getWireframe() const {
	return m_descRasterizerState.FillMode != D3D11_FILL_SOLID;
}

void Renderer::setWireframe(bool bWireframe) {
	if (getWireframe() == bWireframe) return;

	if (bWireframe) {
		m_descRasterizerState.FillMode = D3D11_FILL_WIREFRAME;
		m_descRasterizerState.CullMode = D3D11_CULL_BACK;
	} else {
		m_descRasterizerState.FillMode = D3D11_FILL_SOLID;
		m_descRasterizerState.CullMode = D3D11_CULL_BACK;
	}
	ID3D11RasterizerState* pRS = nullptr;
	m_pDevice->CreateRasterizerState(&m_descRasterizerState, &pRS);
	m_pDeviceContext->RSSetState(pRS);
	pRS->Release();

	if (bWireframe)
		setPostProcessAA(false);
}

void Renderer::setPostProcessAA(bool bPostProcessAA) {
	m_bPostProcessAA = bPostProcessAA;
	if (bPostProcessAA)
		setWireframe(false);
}

void Renderer::dump() {
	m_bDump = true;

	SetWindowText(m_hwnd, _APP_NAME_ _T(" (Capturing Frame...)"));
}

void Renderer::dumpShaderResourceViewToFile(ID3D11ShaderResourceView* pSRV, const TString& strFileName) {
	ID3D11Resource* pTexture2D = nullptr;
	pSRV->GetResource(&pTexture2D);

	dumpTextureToFile(pTexture2D, strFileName);

	pTexture2D->Release();
}

void Renderer::dumpRenderTargetToFile(ID3D11RenderTargetView* pRT, const TString& strFileName) {
	ID3D11Resource* pTexture2D = nullptr;
	pRT->GetResource(&pTexture2D);

	dumpTextureToFile(pTexture2D, strFileName);

	pTexture2D->Release();
}

void Renderer::dumpTextureToFile(ID3D11Resource* pTexture2D, const TString& strFileName) {
	TStringStream tss;
	tss << (++m_nDumpCount) << _T("_") << strFileName << _T(".png");
	SetWindowText(m_hwnd, (_APP_NAME_ _T(" (Capturing ") + tss.str() + _T("...)")).c_str());
	ScratchImage img;
	CaptureTexture(m_pDevice, m_pDeviceContext, pTexture2D, img);
	SaveToWICFile(*img.GetImage(0, 0, 0), WIC_FLAGS_NONE, GUID_ContainerFormatPng, tss.str().c_str());
}

ID3D11Device* Renderer::getDevice() const {
	return m_pDevice;
}

ID3D11DeviceContext* Renderer::getDeviceContext() const {
	return m_pDeviceContext;
}

void Renderer::setMaterial(const Material& mt) {
	m_cbMaterial.g_mtAmbient = XMColor3(mt.coAmbient);
	m_cbMaterial.g_mtDiffuse = XMColor3(mt.coDiffuse);
	m_cbMaterial.g_mtSpecular = XMColor3(mt.coSpecular);
	m_cbMaterial.g_mtEmissive = XMColor3(mt.coEmissive);
	m_cbMaterial.g_mtShininess = mt.fShininess;
	m_cbMaterial.g_mtBumpMultiplier = mt.fBumpMultiplier;
	m_pDeviceContext->UpdateSubresource(m_pMaterialConstantBuffer, 0, NULL, &m_cbMaterial, 0, 0);
}

void Renderer::setWorldMatrix(const XMMATRIX& matWorld) {
	XMStoreFloat4x4(&m_cbTransform.g_matWorld, XMMatrixTranspose(matWorld));
	m_pDeviceContext->UpdateSubresource(m_pTransformConstantBuffer, 0, NULL, &m_cbTransform, 0, 0);
}

void Renderer::useTexture(ID3D11SamplerState* pTextureSamplerState, ID3D11ShaderResourceView* pTexture) {
	m_pDeviceContext->PSSetSamplers(SLOT_TEXTURE, 1, &pTextureSamplerState);
	m_pDeviceContext->PSSetShaderResources(SLOT_TEXTURE, 1, &pTexture);
}

void Renderer::usePlaceholderTexture() {
	m_pDeviceContext->PSSetSamplers(SLOT_TEXTURE, 1, &m_pPlaceholderSamplerState);
	m_pDeviceContext->PSSetShaderResources(SLOT_TEXTURE, 1, &m_pPlaceholderTexture);
}

void Renderer::useBumpMap(ID3D11SamplerState* pBumpMapSamplerState, ID3D11ShaderResourceView* pBumpMap) {
	if (!m_bBump) {
		usePlaceholderBumpMap();
		return;
	}
	m_pDeviceContext->VSSetSamplers(SLOT_BUMPMAP, 1, &pBumpMapSamplerState);
	m_pDeviceContext->VSSetShaderResources(SLOT_BUMPMAP, 1, &pBumpMap);
	m_pDeviceContext->HSSetSamplers(SLOT_BUMPMAP, 1, &pBumpMapSamplerState);
	m_pDeviceContext->HSSetShaderResources(SLOT_BUMPMAP, 1, &pBumpMap);
	m_pDeviceContext->DSSetSamplers(SLOT_BUMPMAP, 1, &pBumpMapSamplerState);
	m_pDeviceContext->DSSetShaderResources(SLOT_BUMPMAP, 1, &pBumpMap);
	m_pDeviceContext->PSSetSamplers(SLOT_BUMPMAP, 1, &pBumpMapSamplerState);
	m_pDeviceContext->PSSetShaderResources(SLOT_BUMPMAP, 1, &pBumpMap);
}

void Renderer::usePlaceholderBumpMap() {
	m_pDeviceContext->VSSetSamplers(SLOT_BUMPMAP, 1, &m_pBumpSamplerState);
	m_pDeviceContext->VSSetShaderResources(SLOT_BUMPMAP, 1, &m_pBumpTexture);
	m_pDeviceContext->HSSetSamplers(SLOT_BUMPMAP, 1, &m_pBumpSamplerState);
	m_pDeviceContext->HSSetShaderResources(SLOT_BUMPMAP, 1, &m_pBumpTexture);
	m_pDeviceContext->DSSetSamplers(SLOT_BUMPMAP, 1, &m_pBumpSamplerState);
	m_pDeviceContext->DSSetShaderResources(SLOT_BUMPMAP, 1, &m_pBumpTexture);
	m_pDeviceContext->PSSetSamplers(SLOT_BUMPMAP, 1, &m_pBumpSamplerState);
	m_pDeviceContext->PSSetShaderResources(SLOT_BUMPMAP, 1, &m_pBumpTexture);
}

void Renderer::useNormalMap(ID3D11SamplerState* pNormalMapSamplerState, ID3D11ShaderResourceView* pNormalMap) {
	if (!m_bBump) {
		usePlaceholderNormalMap();
		return;
	}
	m_pDeviceContext->PSSetSamplers(SLOT_NORMALMAP, 1, &pNormalMapSamplerState);
	m_pDeviceContext->PSSetShaderResources(SLOT_NORMALMAP, 1, &pNormalMap);
}

void Renderer::usePlaceholderNormalMap() {
	m_pDeviceContext->PSSetSamplers(SLOT_NORMALMAP, 1, &m_pBumpSamplerState); // use bump map's point sampler
	m_pDeviceContext->PSSetShaderResources(SLOT_NORMALMAP, 1, &m_pNormalTexture);
}

void Renderer::setTessellationFactor(float edge, float inside, float min, float desiredSizeInPixels) {
	// pass (half screen height / desired size in pixels) to the shader
	float desiredSize = (float)(m_rsCurrent == RS_ShadowMap ? SM_SIZE : m_rectView.Height()) / (2.0f * desiredSizeInPixels);
	m_cbTessellation.g_vTesselationFactor = XMFLOAT4(edge, inside, min, desiredSize);
	m_pDeviceContext->UpdateSubresource(m_pTessellationConstantBuffer, 0, NULL, &m_cbTessellation, 0, 0);
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
