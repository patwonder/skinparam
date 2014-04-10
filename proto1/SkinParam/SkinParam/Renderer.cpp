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

const float Renderer::CLIPPING_LIGHT_NEAR = 0.1f;
const float Renderer::CLIPPING_LIGHT_FAR = 20.0f;
const float Renderer::FOV_LIGHT = 50.0f;
const float Renderer::CLIPPING_SCENE_NEAR = 0.1f;
const float Renderer::CLIPPING_SCENE_FAR = 20.0f;
const float Renderer::FOV_SCENE = 30.0f;
const float Renderer::MM_PER_LENGTH = 80.f;

const TCHAR* Renderer::DEFAULT_DUMP_FOLDER = _T("dump");

Renderer::Renderer(HWND hwnd, CRect rectView, Config* pConfig, Camera* pCamera, RenderableManager* pRenderableManager)
	: m_psgShadow(nullptr),
	  m_psgTessellatedPhong(nullptr),
	  m_psgTessellatedShadow(nullptr),
	  m_psgCanvas(nullptr),
	  m_psgCanvasAA(nullptr),
	  m_psgSSSIrradiance(nullptr),
	  m_psgSSSIrradianceNoTessellation(nullptr),
	  m_psgSSSIrradianceNoGaussian(nullptr),
	  m_psgSSSIrradianceNoGaussianAA(nullptr),
	  m_psgSSSIrradianceNoTessellationNoGaussian(nullptr),
	  m_psgSSSIrradianceNoTessellationNoGaussianAA(nullptr),
	  m_psgSSSGausianVertical(nullptr),
	  m_psgSSSGausianHorizontal(nullptr),
	  m_psgSSSGausianVertical7(nullptr),
	  m_psgSSSGausianHorizontal7(nullptr),
	  m_psgSSSGausianVertical5(nullptr),
	  m_psgSSSGausianHorizontal5(nullptr),
	  m_psgSSSGausianVertical3(nullptr),
	  m_psgSSSGausianHorizontal3(nullptr),
	  m_psgSSSCombine(nullptr),
	  m_psgSSSCombineAA(nullptr),
	  m_psgSSSAttenuationTexture(nullptr),
	  m_psgPostProcessAA(nullptr),
	  m_psgCopy(nullptr),
	  m_psgCopyLinear(nullptr),
	  m_psgBloomDetect(nullptr),
	  m_psgBloomVertical(nullptr),
	  m_psgBloomHorizontal(nullptr),
	  m_psgBloomCombine(nullptr),
	  m_psgBloomCombineAA(nullptr),
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
	  m_bAdaptiveGaussian(true),
	  m_bBloom(true),
	  m_bDump(false),
	  m_rsCurrent(RS_NotRendering),
	  m_bPRAttenuationTexture(false),
	  m_sssGaussianParamsCalculator(_T("model/profilefit_6.txt")),
	  m_sssSkinParams(0, 0, 0, 0)
{
	m_vppShaderGroups.push_back(&m_psgShadow);
	m_vppShaderGroups.push_back(&m_psgTessellatedPhong);
	m_vppShaderGroups.push_back(&m_psgTessellatedShadow);
	m_vppShaderGroups.push_back(&m_psgCanvas);
	m_vppShaderGroups.push_back(&m_psgCanvasAA);
	m_vppShaderGroups.push_back(&m_psgSSSIrradiance);
	m_vppShaderGroups.push_back(&m_psgSSSIrradianceNoTessellation);
	m_vppShaderGroups.push_back(&m_psgSSSIrradianceNoGaussian);
	m_vppShaderGroups.push_back(&m_psgSSSIrradianceNoGaussianAA);
	m_vppShaderGroups.push_back(&m_psgSSSIrradianceNoTessellationNoGaussian);
	m_vppShaderGroups.push_back(&m_psgSSSIrradianceNoTessellationNoGaussianAA);
	m_vppShaderGroups.push_back(&m_psgSSSGausianVertical);
	m_vppShaderGroups.push_back(&m_psgSSSGausianHorizontal);
	m_vppShaderGroups.push_back(&m_psgSSSGausianVertical7);
	m_vppShaderGroups.push_back(&m_psgSSSGausianHorizontal7);
	m_vppShaderGroups.push_back(&m_psgSSSGausianVertical5);
	m_vppShaderGroups.push_back(&m_psgSSSGausianHorizontal5);
	m_vppShaderGroups.push_back(&m_psgSSSGausianVertical3);
	m_vppShaderGroups.push_back(&m_psgSSSGausianHorizontal3);
	m_vppShaderGroups.push_back(&m_psgSSSCombine);
	m_vppShaderGroups.push_back(&m_psgSSSCombineAA);
	m_vppShaderGroups.push_back(&m_psgSSSAttenuationTexture);
	m_vppShaderGroups.push_back(&m_psgPostProcessAA);
	m_vppShaderGroups.push_back(&m_psgCopy);
	m_vppShaderGroups.push_back(&m_psgCopyLinear);
	m_vppShaderGroups.push_back(&m_psgBloomDetect);
	m_vppShaderGroups.push_back(&m_psgBloomVertical);
	m_vppShaderGroups.push_back(&m_psgBloomHorizontal);
	m_vppShaderGroups.push_back(&m_psgBloomCombine);
	m_vppShaderGroups.push_back(&m_psgBloomCombineAA);

	m_sssGaussianParams = m_sssGaussianParamsCalculator.getParams(m_sssSkinParams);

	checkFailure(initDX(), _T("Failed to initialize DirectX 11"));

	initMisc();

	loadShaders();
	initTransform();
	initLighting();
	initMaterial();
	updateProjection();
	initTessellation();
	initShadowMaps();
	initCopy();
	initViewInView();

	initSSS();
	initBloom();
	initPostProcessAA();

	doPreRenderings();
}

Renderer::~Renderer() {
	removeAllRenderables();
	unloadShaders();

	m_pRenderableManager->onReleasingSwapChain();
	m_pRenderableManager->onDestroyDevice();

	if (m_pDeviceContext)
		m_pDeviceContext->ClearState();
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
    ds.d3d11.CreateFlags = createDeviceFlags;
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

	// Use lower presets for non-hardware devices
	if (m_driverType != D3D_DRIVER_TYPE_HARDWARE) {
		m_bTessellation = false;
		m_bPostProcessAA = false;
		m_bBloom = false;
	}

	m_pSwapChain = DXUTGetDXGISwapChain();
	m_pDevice = DXUTGetD3D11Device();
	m_featureLevel = DXUTGetD3D11DeviceFeatureLevel();
	m_pDeviceContext = DXUTGetD3D11DeviceContext();

	m_pRenderableManager->onCreateDevice(m_pDevice, m_pDeviceContext, m_pSwapChain);
	
	const DXGI_SURFACE_DESC* pDesc = DXUTGetDXGIBackBufferSurfaceDesc();
	m_rectView = CRect(0, 0, pDesc->Width, pDesc->Height);
	m_pRenderableManager->onResizedSwapChain(m_pDevice, pDesc);

    // Create a render target view
    CComPtr<ID3D11Texture2D> pBackBuffer;
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return hr;

	hr = m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView);
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

	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView.p, m_pDepthStencilView);

	// Setup the viewport
	m_vpScreen = createViewport(m_rectView.Width(), m_rectView.Height(), m_rectView.left, m_rectView.top);
    m_pDeviceContext->RSSetViewports(1, &m_vpScreen);

	// Create default rasterizer state
	D3D11_RASTERIZER_DESC& desc = m_descRasterizerState;
	ZeroMemory(&desc, sizeof(desc));
	desc.FillMode = D3D11_FILL_SOLID;
	desc.CullMode = D3D11_CULL_BACK;
	desc.DepthClipEnable = TRUE;
	CComPtr<ID3D11RasterizerState> pRS;
	m_pDevice->CreateRasterizerState(&desc, &pRS);
	m_pDeviceContext->RSSetState(pRS);

    return S_OK;
}

void Renderer::initMisc() {
	// Creates the placeholder texture for non-textured objects
	checkFailure(createSamplerState(m_pDevice, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, &m_pPlaceholderSamplerState),
		_T("Failed to create placeholder sampler state"));

	checkFailure(createShaderResourceView2D(m_pDevice, 1, 1, DXGI_FORMAT_R32_FLOAT, &m_pPlaceholderTexture),
		_T("Failed to create placeholder texture"));

	{
		float initData[] = { 1.0f };
		CComPtr<ID3D11Resource> pResource;
		m_pPlaceholderTexture->GetResource(&pResource);
		m_pDeviceContext->UpdateSubresource(pResource, 0, nullptr, initData, 4, 0);
	}

	// Creates the placeholder bump texture for non-bumpmapped objects
	checkFailure(createSamplerState(m_pDevice, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, &m_pBumpSamplerState),
		_T("Failed to create placeholder bump sampler state"));

	checkFailure(createShaderResourceView2D(m_pDevice, 1, 1, DXGI_FORMAT_R32_FLOAT, &m_pBumpTexture),
		_T("Failed to create placeholder bump map"));

	{
		float initData[] = { 0.5f };
		CComPtr<ID3D11Resource> pResource;
		m_pBumpTexture->GetResource(&pResource);
		m_pDeviceContext->UpdateSubresource(pResource, 0, nullptr, initData, 4, 0);
	}

	// Creates the placeholder normal texture for non-normalmapped objects
	checkFailure(createShaderResourceView2D(m_pDevice, 1, 1, DXGI_FORMAT_R32G32_FLOAT, &m_pNormalTexture),
		_T("Failed to create placeholder normal map"));
	{
		float initData[] = { 0.0f, 0.0f };
		CComPtr<ID3D11Resource> pResource;
		m_pNormalTexture->GetResource(&pResource);
		m_pDeviceContext->UpdateSubresource(pResource, 0, nullptr, initData, 8, 0);
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
	D3D11_INPUT_ELEMENT_DESC phong_layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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
	m_psgSSSIrradianceNoGaussian = new ShaderGroup(m_pDevice, _T("TessellatedPhong.fx"), phong_layout, ARRAYSIZE(phong_layout),
		"VS", "HS", "DS_Irradiance", "PS_Irradiance_NoGaussian");
	m_psgSSSIrradianceNoGaussianAA = new ShaderGroup(m_pDevice, _T("TessellatedPhong.fx"), phong_layout, ARRAYSIZE(phong_layout),
		"VS", "HS", "DS_Irradiance", "PS_Irradiance_NoGaussian_AA");
	m_psgSSSIrradianceNoTessellationNoGaussian = new ShaderGroup(m_pDevice, _T("TessellatedPhong.fx"), phong_layout, ARRAYSIZE(phong_layout),
		"VS_Irradiance_NoTessellation", nullptr, nullptr, "PS_Irradiance_NoGaussian");
	m_psgSSSIrradianceNoTessellationNoGaussianAA = new ShaderGroup(m_pDevice, _T("TessellatedPhong.fx"), phong_layout, ARRAYSIZE(phong_layout),
		"VS_Irradiance_NoTessellation", nullptr, nullptr, "PS_Irradiance_NoGaussian_AA");

	// Canvas-space shaders
	D3D11_INPUT_ELEMENT_DESC canvas_layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	m_psgCanvas = new ShaderGroup(m_pDevice, _T("Canvas.fx"), canvas_layout, ARRAYSIZE(canvas_layout),
		"VS", nullptr, nullptr, "PS");
	m_psgCanvasAA = new ShaderGroup(m_pDevice, _T("Canvas.fx"), canvas_layout, ARRAYSIZE(canvas_layout),
		"VS", nullptr, nullptr, "PS_AA");

	// Screen-space shaders
	D3D11_INPUT_ELEMENT_DESC empty_layout[] = { { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 } };
	// Copy shader
	m_psgCopy = new ShaderGroup(m_pDevice, _T("Copy.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS");
	m_psgCopyLinear = new ShaderGroup(m_pDevice, _T("Copy.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS_Linear");

	// Subsurface Scattering
	m_psgSSSGausianVertical = new ShaderGroup(m_pDevice, _T("Gaussian.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS_Vertical");
	m_psgSSSGausianHorizontal = new ShaderGroup(m_pDevice, _T("Gaussian.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS_Horizontal");
	m_psgSSSGausianVertical7 = new ShaderGroup(m_pDevice, _T("Gaussian.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS_Vertical_7");
	m_psgSSSGausianHorizontal7 = new ShaderGroup(m_pDevice, _T("Gaussian.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS_Horizontal_7");
	m_psgSSSGausianVertical5 = new ShaderGroup(m_pDevice, _T("Gaussian.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS_Vertical_5");
	m_psgSSSGausianHorizontal5 = new ShaderGroup(m_pDevice, _T("Gaussian.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS_Horizontal_5");
	m_psgSSSGausianVertical3 = new ShaderGroup(m_pDevice, _T("Gaussian.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS_Vertical_3");
	m_psgSSSGausianHorizontal3 = new ShaderGroup(m_pDevice, _T("Gaussian.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS_Horizontal_3");
	m_psgSSSCombine = new ShaderGroup(m_pDevice, _T("Combine.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS");
	m_psgSSSCombineAA = new ShaderGroup(m_pDevice, _T("Combine.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS_AA");

	// Post-process Anti-aliasing
	m_psgPostProcessAA = new ShaderGroup(m_pDevice, _T("PostProcessAA.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS");

	// Bloom filter
	m_psgBloomDetect = new ShaderGroup(m_pDevice, _T("Bloom.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS_Detect");
	m_psgBloomVertical = new ShaderGroup(m_pDevice, _T("Bloom.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS_Vertical");
	m_psgBloomHorizontal = new ShaderGroup(m_pDevice, _T("Bloom.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS_Horizontal");
	m_psgBloomCombine = new ShaderGroup(m_pDevice, _T("BloomCombine.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS");
	m_psgBloomCombineAA = new ShaderGroup(m_pDevice, _T("BloomCombine.fx"), empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS_AA");
}

void Renderer::unloadShaders() {
	for (ShaderGroup** ppsg : m_vppShaderGroups) {
		if (*ppsg)
			delete *ppsg;
		*ppsg = nullptr;
	}

	m_vppShaderGroups.clear();
	D3DHelper::clearShaderCache();
}

void Renderer::initViewInView() {
	// initialize view-in-view viewport
	UINT vpsize = m_rectView.Width() / 4;
	m_vpViewInView = createViewport(vpsize, vpsize, m_rectView.right - vpsize, m_rectView.bottom - vpsize);

	// initialize view-in-view vertex buffer
	XMFLOAT2 rcpVPSizeHalf = XMFLOAT2(0.5f / vpsize, 0.5f / vpsize);
	CanvasVertex vertices[] = {
		// black background quad
		{ XMFLOAT3(0.00f, 0.00f, 0.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.00f, 0.00f, 0.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.00f, 1.00f, 0.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.00f, 1.00f, 0.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
		// White border lines
		{ XMFLOAT3(0.00f, rcpVPSizeHalf.y, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.00f, rcpVPSizeHalf.y, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(rcpVPSizeHalf.x, 0.00f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(rcpVPSizeHalf.x, 1.00f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
	};
	checkFailure(createVertexBuffer(m_pDevice, vertices, ARRAYSIZE(vertices), &m_pVIVVertexBuffer),
		_T("Failed to create VIV vertex buffer"));
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

Color Renderer::getGlobalAmbient() const {
	const XMFLOAT3& a = m_cbLighting.g_ambient;
	return Color(a.x, a.y, a.z, 1.0f);
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
		checkFailure(createIntermediateRenderTarget(m_pDevice, SM_SIZE, SM_SIZE, DXGI_FORMAT_R32_FLOAT,
			nullptr, &m_apSRVShadowMaps[i], &m_apRTShadowMaps[i]),
			_T("Failed to create intermediate render target for shadow map"));
	}

	// Create depth stencil view for shadow maps
	CComPtr<ID3D11Texture2D> pTexture2D;
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

	// Create shadow map sampler state (clamp to border with minimum L vector length)
	checkFailure(createSamplerComparisonStateEx(m_pDevice, D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
		D3D11_TEXTURE_ADDRESS_BORDER, D3D11_TEXTURE_ADDRESS_BORDER, D3D11_TEXTURE_ADDRESS_BORDER,
		D3D11_COMPARISON_LESS, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f), 0, &m_pShadowMapSamplerState),
		_T("Failed to create shadow map sampler comparison state"));
	checkFailure(createSamplerStateEx(m_pDevice, D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_TEXTURE_ADDRESS_BORDER, D3D11_TEXTURE_ADDRESS_BORDER, D3D11_TEXTURE_ADDRESS_BORDER,
		XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f), 0, &m_pShadowMapDepthSamplerState),
		_T("Failed to create shadow map depth sampler state"));

	// Initialize shadow map view port
	m_vpShadowMap = createViewport(SM_SIZE, SM_SIZE);
}

void Renderer::bindShadowMaps() {
	// use trilinear comparison sampler (clamp to border) for shadow maps
	m_pDeviceContext->PSSetSamplers(SLOT_SHADOWMAP, 1, &m_pShadowMapSamplerState.p);
	// use point sampler (clamp to border) for shadow depth
	m_pDeviceContext->PSSetSamplers(SLOT_SHADOWMAPDEPTH, 1, &m_pShadowMapDepthSamplerState.p);

	m_pDeviceContext->PSSetShaderResources(SLOT_SHADOWMAP, NUM_LIGHTS, &m_apSRVShadowMaps[0].p);
}

void Renderer::unbindShadowMaps() {
	ID3D11ShaderResourceView* pNullSRVs[NUM_LIGHTS] = { nullptr };
	m_pDeviceContext->PSSetShaderResources(SLOT_SHADOWMAP, NUM_LIGHTS, pNullSRVs);
}

void Renderer::initSSS() {
	for (UINT i = 0; i < NUM_SSS_VIEWS; i++) {
		DXGI_FORMAT format = (i == IDX_SSS_DIFFUSE_STENCIL) ? DXGI_FORMAT_R32G32_FLOAT : DXGI_FORMAT_R32G32B32A32_FLOAT;
		checkFailure(createIntermediateRenderTarget(m_pDevice, m_rectView.Width(), m_rectView.Height(), format,
			nullptr, &m_apSRVSSS[i], &m_apRTSSS[i]),
			_T("Failed to create intermediate render target for SSS"));
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
		m_cbCombine.g_weights[i].value = m_sssGaussianParams.coeffs[i];
	}
	checkFailure(createConstantBuffer(m_pDevice, &m_cbCombine, &m_pCombineConstantBuffer),
		_T("Failed to create combine constant buffer"));

	m_cbSSS.g_sss_intensity = 0.0f;
	m_cbSSS.g_sss_strength = 1.0f;
	updateSSSConstantBufferForParams();
	checkFailure(createConstantBuffer(m_pDevice, &m_cbSSS, &m_pSSSConstantBuffer),
		_T("Failed to create SSS constant buffer"));

	// initalize attenuation texture & viewport
	// first try to load it from file
	HRESULT hr = loadSRVFromWICFile(m_pDevice, m_pDeviceContext, _T("atten.png"), &m_pSRVAttenuationTexture);
	if (FAILED(hr)) {
		// not exists, try to compute it on the fly
		MessageBox(m_hwnd, _T("Attenuation texture not found. Will compute it on the fly.\nThis might take a few seconds."),
			_T("Warning"), MB_OK | MB_ICONWARNING);
		D3D11_INPUT_ELEMENT_DESC empty_layout;
		m_psgSSSAttenuationTexture = new ShaderGroup(m_pDevice, _T("AttenuationTexture.fx"), &empty_layout, 0, "VS_Quad", nullptr, nullptr, "PS");
		checkFailure(createIntermediateRenderTarget(m_pDevice, SSS_ATTENUATION_TEXTURE_SIZE, SSS_ATTENUATION_TEXTURE_SIZE, DXGI_FORMAT_R16_UNORM,
			nullptr, &m_pSRVAttenuationTexture, &m_pRTAttenuationTexture),
			_T("Failed to create intermediate render target for attenuation texture"));

		m_bPRAttenuationTexture = true;

		m_vpAttenuationTexture = createViewport(SSS_ATTENUATION_TEXTURE_SIZE, SSS_ATTENUATION_TEXTURE_SIZE);
	}
}

void Renderer::initBloom() {
	m_cbBloom.rcpScreenSize = XMFLOAT2(1.0f / m_rectView.Width(), 1.0f / m_rectView.Height());
	m_cbBloom.sampleLevel = 0;
	checkFailure(createConstantBuffer(m_pDevice, &m_cbBloom, &m_pBloomConstantBuffer),
		_T("Failed to create bloom constant buffer"));

	checkFailure(createIntermediateRenderTarget(m_pDevice, m_rectView.Width(), m_rectView.Height(), DXGI_FORMAT_R32G32B32A32_FLOAT,
		nullptr, &m_pSRVBloomSource, &m_pRTBloomSource),
		_T("Failed to create bloom source texture"));

	checkFailure(createIntermediateRenderTargetEx(m_pDevice, m_rectView.Width(), m_rectView.Height(), DXGI_FORMAT_R32G32B32A32_FLOAT, false, 1, 0,
		nullptr, &m_pSRVBloomDetect, &m_pRTBloomDetect),
		_T("Failed to create bloom detect texture"));

	for (UINT i = 1; i < BLOOM_PASSES; i++) {
		UINT width = m_rectView.Width() >> i;
		UINT height = m_rectView.Height() >> i;
		checkFailure(createIntermediateRenderTarget(m_pDevice, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT,
			nullptr, &m_apSRVBloom[i], &m_apRTBloom[i]),
			_T("Failed to create bloom target texture"));
		checkFailure(createIntermediateRenderTarget(m_pDevice, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT,
			nullptr, &m_apSRVBloomTemp[i], &m_apRTBloomTemp[i]),
			_T("Failed to create bloom temporary texture"));

		m_avpBloom[i] = createViewport(width, height);
	}

	D3D11_BLEND_DESC desc;
	desc.AlphaToCoverageEnable = FALSE;
	desc.IndependentBlendEnable = FALSE;
	desc.RenderTarget[0].BlendEnable = FALSE;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	checkFailure(m_pDevice->CreateBlendState(&desc, &m_pBSNoBlending),
		_T("Failed to create blending state NoBlending"));

	desc.RenderTarget[0].BlendEnable = TRUE;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_DEST_ALPHA;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	checkFailure(m_pDevice->CreateBlendState(&desc, &m_pBSAdditiveBlending),
		_T("Failed to create blending state NoBlending"));
}

void Renderer::initPostProcessAA() {
	m_cbPostProcess.rcpFrame = XMFLOAT2(1.0f / m_rectView.Width(), 1.0f / m_rectView.Height());
	m_cbPostProcess.rcpFrameOpt = XMFLOAT4(2.0f / m_rectView.Width(), 2.0f / m_rectView.Height(), 0.5f / m_rectView.Width(), 0.5f / m_rectView.Height());

	checkFailure(createConstantBuffer(m_pDevice, &m_cbPostProcess, &m_pPostProcessConstantBuffer),
		_T("Failed to create post-process constant buffer"));
}

void Renderer::initCopy() {
	m_cbCopy.scaleFactor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_cbCopy.defaultValue = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_cbCopy.lerps = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	checkFailure(createConstantBuffer(m_pDevice, &m_cbCopy, &m_pCopyConstantBuffer),
		_T("Failed to create copy constant buffer"));
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

void Renderer::getImmediateIrradianceRT(ID3D11RenderTargetView** ppRT, ShaderGroup** ppsg, float* palpha) {
	if (palpha)
		*palpha = 1.0f;
	if (m_bBloom) {
		if (ppsg)
			*ppsg = m_bTessellation ? m_psgSSSIrradianceNoGaussian : m_psgSSSIrradianceNoTessellationNoGaussian;
		if (ppRT)
			*ppRT = m_pRTBloomSource;
	} else if (m_bPostProcessAA) {
		if (ppsg)
			*ppsg = m_bTessellation ? m_psgSSSIrradianceNoGaussianAA : m_psgSSSIrradianceNoTessellationNoGaussianAA;
		if (ppRT)
			*ppRT = m_apRTSSS[IDX_SSS_TEMPORARY];
		if (palpha)
			*palpha = 0.0f; // for luminance
	} else {
		if (ppsg)
			*ppsg = m_bTessellation ? m_psgSSSIrradianceNoGaussian : m_psgSSSIrradianceNoTessellationNoGaussian;
		if (ppRT)
			*ppRT = m_pRenderTargetView;
	}
}

void Renderer::renderIrradianceMap(bool* opbNeedBlur) {
	setConstantBuffers();

	m_pDeviceContext->IASetPrimitiveTopology(
		m_bTessellation ? D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	bool bUseFastRoutine = opbNeedBlur && !*opbNeedBlur;
	ID3D11RenderTargetView* pRT = nullptr;
	if (!bUseFastRoutine) {
		// Render the irradiance map
		(m_bTessellation ? m_psgSSSIrradiance : m_psgSSSIrradianceNoTessellation)->use(m_pDeviceContext);

		ID3D11RenderTargetView* apSSSRenderTargets[] = {
			m_apRTSSS[IDX_SSS_IRRADIANCE],
			m_apRTSSS[IDX_SSS_ALBEDO],
			m_apRTSSS[IDX_SSS_DIFFUSE_STENCIL],
			m_apRTSSS[IDX_SSS_SPECULAR]
		};
		m_pDeviceContext->OMSetRenderTargets(ARRAYSIZE(apSSSRenderTargets), apSSSRenderTargets, m_pDepthStencilView);
		// Clear render target view & depth stencil
		float ClearIrradiance[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // RGBA
		float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // RGBA
		float ClearDiffuseStencil[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
		m_pDeviceContext->ClearRenderTargetView(m_apRTSSS[IDX_SSS_IRRADIANCE], ClearIrradiance);
		m_pDeviceContext->ClearRenderTargetView(m_apRTSSS[IDX_SSS_ALBEDO], ClearColor);
		m_pDeviceContext->ClearRenderTargetView(m_apRTSSS[IDX_SSS_DIFFUSE_STENCIL], ClearDiffuseStencil);
		m_pDeviceContext->ClearRenderTargetView(m_apRTSSS[IDX_SSS_SPECULAR], ClearColor);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	} else {
		// fast routine, render directly to target
		ShaderGroup* sg = nullptr;
		float alpha = 1.0f;
		getImmediateIrradianceRT(&pRT, &sg, &alpha);
		sg->use(m_pDeviceContext);

		m_pDeviceContext->OMSetRenderTargets(1, &pRT, m_pDepthStencilView);
		// Clear render target view & depth stencil
		float ClearColor[4] = { 0.0f, 0.0f, 0.0f, alpha }; // RGBA
		m_pDeviceContext->ClearRenderTargetView(pRT, ClearColor);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	updateTransform();
	bindShadowMaps();
	bindAttenuationTexture();
	renderScene(opbNeedBlur);
	unbindAttenuationTexture();
	unbindShadowMaps();
	unbindInputBuffers();

	if (m_bDump) {
		if (!bUseFastRoutine) {
			dumpResourceToFile(m_apRTSSS[IDX_SSS_IRRADIANCE], _T("Irradiance"), false, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
			dumpResourceToFile(m_apRTSSS[IDX_SSS_ALBEDO], _T("Albedo"), false, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
			dumpIrregularResourceToFile(m_apRTSSS[IDX_SSS_DIFFUSE_STENCIL], _T("DiffuseStencil"), false,
				XMFLOAT4(50.0f, 50.0f, 0.0f, 0.0f),
				XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
				XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f),
				DXGI_FORMAT_R16G16B16A16_UNORM);
			dumpResourceToFile(m_apRTSSS[IDX_SSS_SPECULAR], _T("Specular"), false, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		} else {
			if (!m_bPostProcessAA && !m_bBloom)
				dumpResourceToFile(pRT, _T("Final"), false, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		}
	}
}

void Renderer::renderViewInView(const Camera* pCamera) {
	bool bAA = !m_bBloom && m_bPostProcessAA;
	ShaderGroup* sgcanvas = bAA ? m_psgCanvasAA : m_psgCanvas;

	ID3D11RenderTargetView* pRT = nullptr;
	ShaderGroup* sg = nullptr;
	float alpha = 1.0f;
	getImmediateIrradianceRT(&pRT, &sg, &alpha);

	// Draw background with depth buffer off
	m_pDeviceContext->RSSetViewports(1, &m_vpViewInView);

	UINT stride = sizeof(CanvasVertex);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVIVVertexBuffer.p, &stride, &offset);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	sgcanvas->use(m_pDeviceContext);
	m_pDeviceContext->OMSetRenderTargets(1, &pRT, nullptr);

	m_pDeviceContext->Draw(4, 0);

	// Draw border with depth buffer on
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_pDeviceContext->OMSetRenderTargets(1, &pRT, m_pDepthStencilView);
	// Clear depth stencil view only
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	m_pDeviceContext->Draw(4, 4);

	// Draw stuff
	setConstantBuffers();

	m_pDeviceContext->IASetPrimitiveTopology(
		m_bTessellation ? D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	sg->use(m_pDeviceContext);
	
	updateTransform(*pCamera, XMLoadFloat4x4(&m_matLightProjection), (float)(SM_SIZE / SM_SIZE));
	bindShadowMaps();
	bindAttenuationTexture();
	renderScene();
	unbindAttenuationTexture();
	unbindShadowMaps();
	unbindInputBuffers();

	m_pDeviceContext->RSSetViewports(1, &m_vpScreen);
}

void Renderer::unbindInputBuffers() {
	// unbind all vertex and index buffers
	UINT zero = 0;
	ID3D11Buffer* nullBuffer = nullptr;
	m_pDeviceContext->IASetVertexBuffers(0, 1, &nullBuffer, &zero, &zero);
	m_pDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
}

void Renderer::bindGaussianConstantBuffer() {
	m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pGaussianConstantBuffer.p);
}

void Renderer::bindCombineConstantBuffer() {
	m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pCombineConstantBuffer.p);
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
	// 7-tap to 5-tap limit
	static const float SEVEN_TO_FIVE = 0.873f;
	// 5-tap to 3-tap limit
	static const float FIVE_TO_THREE = 0.593f;
	// 3-tap to copy limit
	static const float THREE_TO_ONE = 0.1f;

	if (kernelSizeInPixels > SEVEN_TO_FIVE) {
		// standard 7 tap filter
		*ppsgVertical = m_psgSSSGausianVertical7;
		*ppsgHorizontal = m_psgSSSGausianHorizontal7;
	} else if (kernelSizeInPixels > FIVE_TO_THREE) {
		*ppsgVertical = m_psgSSSGausianVertical5;
		*ppsgHorizontal = m_psgSSSGausianHorizontal5;
	} else if (kernelSizeInPixels > THREE_TO_ONE) {
		*ppsgVertical = m_psgSSSGausianVertical3;
		*ppsgHorizontal = m_psgSSSGausianHorizontal3;
	} else {
		// just copy pixels
		*ppsgVertical = m_psgCopy;
		*ppsgHorizontal = m_psgCopy;
		return true;
	}
	return false;
}

void Renderer::doGaussianBlurs(ID3D11ShaderResourceView* oapSRVGaussians[]) {
	bindGaussianConstantBuffer();
	
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->PSSetSamplers(0, 1, &m_pLinearSamplerState.p);
	m_pDeviceContext->PSSetSamplers(1, 1, &m_pBumpSamplerState.p);

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

	float minDepth = m_bAdaptiveGaussian ? getMinDepthForScene() : CLIPPING_SCENE_NEAR;
	float prevVariance = 0;
	for (UINT i = 0; i < NUM_SSS_GAUSSIANS; i++) {
		float variance = m_sssGaussianParams.sigmas[i] * m_sssGaussianParams.sigmas[i];
		float size = sqrt(variance - prevVariance);

		setGaussianKernelSize(size);

		ShaderGroup* psgGaussianVertical;
		ShaderGroup* psgGaussianHorizontal;
		bool bCopy;
		if (!m_bAdaptiveGaussian) {
			psgGaussianVertical = m_psgSSSGausianVertical;
			psgGaussianHorizontal = m_psgSSSGausianHorizontal;
			bCopy = false;
		} else {
			float kernelSizeInPixels = estimateGaussianKernelSize(size, minDepth);
			bCopy = selectShaderGroupsForKernelSize(kernelSizeInPixels, &psgGaussianVertical, &psgGaussianHorizontal);
		}
		if (bCopy) {
			// use previous texture directly
			oapSRVGaussians[i] = pSRVPrevious;
		} else {
			prevVariance = variance;

			// do separate gaussian passes
			// previous ->(vertical blur)-> temporary
			psgGaussianVertical->use(m_pDeviceContext);
			m_pDeviceContext->OMSetRenderTargets(1, &pRTTemporary, nullptr);
			apSRVs[0] = pSRVPrevious;
			m_pDeviceContext->PSSetShaderResources(0, numViews, apSRVs);

			m_pDeviceContext->Draw(6, 0);

			// temporary ->(horizontal blur)-> next
			psgGaussianHorizontal->use(m_pDeviceContext);
			m_pDeviceContext->OMSetRenderTargets(1, &m_apRTSSS[IDX_SSS_GAUSSIANS_START + i].p, nullptr);
			apSRVs[0] = pSRVTemporary;
			m_pDeviceContext->PSSetShaderResources(0, numViews, apSRVs);

			m_pDeviceContext->Draw(6, 0);

			// must unbind shader resource
			m_pDeviceContext->PSSetShaderResources(0, numViews, apNullSRVs);

			oapSRVGaussians[i] = pSRVPrevious = m_apSRVSSS[IDX_SSS_GAUSSIANS_START + i];
		}

		if (m_bDump) {
			TStringStream tss;
			tss << _T("Gaussian_") << i + 1;
			dumpResourceToFile(pSRVPrevious, tss.str(), false, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		}
	}
}

void Renderer::doCombineShading(bool bNeedBlur, ID3D11ShaderResourceView* apSRVGaussians[]) {
	bindCombineConstantBuffer();

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->PSSetSamplers(0, 1, &m_pLinearSamplerState.p);
	m_pDeviceContext->PSSetSamplers(1, 1, &m_pBumpSamplerState.p);

	((m_bPostProcessAA && !m_bBloom) ? m_psgSSSCombineAA : m_psgSSSCombine)->use(m_pDeviceContext);

	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // RGBA

	// bind the final render target (before AA & bloom)
	ID3D11RenderTargetView* pRenderTargetView = (m_bBloom ? m_pRTBloomSource : (m_bPostProcessAA ? m_apRTSSS[IDX_SSS_TEMPORARY] : m_pRenderTargetView));
	m_pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);

	// Set up shader resources
	const UINT numViews = 2 + NUM_SSS_GAUSSIANS;
	ID3D11ShaderResourceView* apSSSSRVs[numViews] = {
		m_apSRVSSS[IDX_SSS_ALBEDO],
		m_apSRVSSS[IDX_SSS_SPECULAR],
	};
	if (bNeedBlur)
		memcpy(apSSSSRVs + 2, apSRVGaussians, sizeof(*apSRVGaussians) * NUM_SSS_GAUSSIANS);
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

	if (m_bDump && !m_bPostProcessAA && !m_bBloom) {
		dumpResourceToFile(pRenderTargetView, _T("Final"), false, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	}
}

void Renderer::bindPostProcessConstantBuffer() {
	m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pPostProcessConstantBuffer.p);
}

void Renderer::doPostProcessAA() {
	bindPostProcessConstantBuffer();

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->PSSetSamplers(0, 1, &m_pLinearSamplerState.p);
	m_pDeviceContext->PSSetSamplers(1, 1, &m_pBumpSamplerState.p);

	m_psgPostProcessAA->use(m_pDeviceContext);

	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // RGBA

	// bind the (hopefully) final render target
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView.p, nullptr);

	// Set up shader resources, the frame is now in the SSS temporary render target
	m_pDeviceContext->PSSetShaderResources(0, 1, &m_apSRVSSS[IDX_SSS_TEMPORARY].p);

	// do it~
	m_pDeviceContext->Draw(6, 0);

	// unbind all shader resources
	ID3D11ShaderResourceView* pNullSRV = nullptr;
	m_pDeviceContext->PSSetShaderResources(0, 1, &pNullSRV);

	if (m_bDump) {
		dumpResourceToFile(m_pRenderTargetView, _T("Final"), false, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	}
}

void Renderer::doBloom() {
	m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pBloomConstantBuffer.p);

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->PSSetSamplers(0, 1, &m_pLinearSamplerState.p);
	m_pDeviceContext->PSSetSamplers(1, 1, &m_pBumpSamplerState.p);
	
	// Detect bloom area
	m_cbBloom.rcpScreenSize = XMFLOAT2(1.0f / m_rectView.Width(), 1.0f / m_rectView.Height());
	m_pDeviceContext->UpdateSubresource(m_pBloomConstantBuffer, 0, nullptr, &m_cbBloom, 0, 0);

	m_psgBloomDetect->use(m_pDeviceContext);
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRTBloomDetect.p, nullptr);
	m_pDeviceContext->PSSetShaderResources(0, 1, &m_pSRVBloomSource.p);

	m_pDeviceContext->Draw(6, 0);

	if (m_bDump)
		dumpResourceToFile(m_pRTBloomDetect, _T("BloomDetect"), false, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

	// Generate mip-maps for shader use
	m_pDeviceContext->GenerateMips(m_pSRVBloomDetect);

	// for unbinding shader resource views
	ID3D11ShaderResourceView* apNullSRVs[BLOOM_PASSES + 1] = { nullptr };

	// m_pSRVBloomDetect -> m_apRTBloom[1~BLOOM_PASSES-1]
	for (UINT i = 1; i < BLOOM_PASSES; i++) {
		// set the viewport
		m_pDeviceContext->RSSetViewports(1, &m_avpBloom[i]);

		// setup the constant buffer
		m_cbBloom.rcpScreenSize = XMFLOAT2(1.0f / (m_rectView.Width() >> i), 1.0f / (m_rectView.Height() >> i));
		m_cbBloom.sampleLevel = i;
		m_pDeviceContext->UpdateSubresource(m_pBloomConstantBuffer, 0, nullptr, &m_cbBloom, 0, 0);

		// m_pSRVBloomDetect[mip level i] ->(vertical blur)-> m_apRTBloomTemp[i]
		m_psgBloomVertical->use(m_pDeviceContext);
		m_pDeviceContext->OMSetRenderTargets(1, &m_apRTBloomTemp[i].p, nullptr);
		m_pDeviceContext->PSSetShaderResources(0, 1, &m_pSRVBloomDetect.p);

		m_pDeviceContext->Draw(6, 0);

		// m_apSRVBloomTemp[i] ->(horizontal blur)-> m_apRTBloom[i]
		m_psgBloomHorizontal->use(m_pDeviceContext);
		m_pDeviceContext->OMSetRenderTargets(1, &m_apRTBloom[i].p, nullptr);
		m_pDeviceContext->PSSetShaderResources(0, 1, &m_apSRVBloomTemp[i].p);

		m_pDeviceContext->Draw(6, 0);

		m_pDeviceContext->PSSetShaderResources(0, 1, apNullSRVs);

		if (m_bDump) {
			TStringStream tss;
			tss << _T("Bloom_") << i;
			dumpResourceToFile(m_apRTBloom[i], tss.str(), false, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		}
	}

	m_pDeviceContext->RSSetViewports(1, &m_vpScreen);

	// m_pSRVBloomSource + m_apSRVBloom[1~BLOOM_PASSES-1] ->(combine)-> (final / post process)
	(m_bPostProcessAA ? m_psgBloomCombineAA : m_psgBloomCombine)->use(m_pDeviceContext);
	m_pDeviceContext->OMSetRenderTargets(1, m_bPostProcessAA ? &m_apRTSSS[IDX_SSS_TEMPORARY].p : &m_pRenderTargetView.p, nullptr);
	ID3D11ShaderResourceView* apSRVs[BLOOM_PASSES + 1] = { m_pSRVBloomSource };
	for (UINT i = 0; i < BLOOM_PASSES; i++)
		apSRVs[i + 1] = m_apSRVBloom[i];

	m_pDeviceContext->PSSetShaderResources(0, BLOOM_PASSES + 1, apSRVs);
	m_pDeviceContext->Draw(6, 0);

	m_pDeviceContext->PSSetShaderResources(0, BLOOM_PASSES + 1, apNullSRVs);

	if (m_bDump && !m_bPostProcessAA)
		dumpResourceToFile(m_pRenderTargetView, _T("Final"), false, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
}


void Renderer::copyRender(ID3D11ShaderResourceView* pSRV, ID3D11RenderTargetView* pRT, bool bLinear,
						  XMFLOAT4 scaleFactor, XMFLOAT4 defaultValue, XMFLOAT4 lerps)
{
	bool bWireframe = getWireframe();
	if (bWireframe) setWireframe(false);

	// Change viewport
	UINT width, height;
	getResourceSize(pRT, &width, &height);
	D3D11_VIEWPORT vpOld, vpNew;
	vpNew = createViewport(width, height);
	UINT numViewports = 1;
	m_pDeviceContext->RSGetViewports(&numViewports, &vpOld);
	m_pDeviceContext->RSSetViewports(1, &vpNew);

	m_cbCopy.scaleFactor = scaleFactor;
	m_cbCopy.defaultValue = defaultValue;
	m_cbCopy.lerps = lerps;
	m_pDeviceContext->UpdateSubresource(m_pCopyConstantBuffer, 0, nullptr, &m_cbCopy, 0, 0);
	m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pCopyConstantBuffer.p);

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->PSSetSamplers(0, 1, &m_pLinearSamplerState.p);
	m_pDeviceContext->PSSetSamplers(1, 1, &m_pBumpSamplerState.p);

	(bLinear ? m_psgCopyLinear : m_psgCopy)->use(m_pDeviceContext);
	m_pDeviceContext->OMSetRenderTargets(1, &pRT, nullptr);
	m_pDeviceContext->PSSetShaderResources(0, 1, &pSRV);

	m_pDeviceContext->Draw(6, 0);

	ID3D11ShaderResourceView* pNullSRV = nullptr;
	m_pDeviceContext->PSSetShaderResources(0, 1, &pNullSRV);

	// Change viewport back
	m_pDeviceContext->RSSetViewports(1, &vpOld);

	if (bWireframe) setWireframe(true);
}

void Renderer::render(const Camera* pSecondaryView) {
	updateLighting();
	updateTessellation();
	setConstantBuffers();

	if (m_bDump)
		m_nDumpCount = 0;

	bool bToggle = getWireframe();
	bool bNeedBlur = false;
	bool bLightLit = false;
	// Render shadow maps
	m_rsCurrent = RS_ShadowMap;
    m_pDeviceContext->IASetPrimitiveTopology(
		m_bTessellation ? D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->RSSetViewports(1, &m_vpShadowMap);
	(m_bTessellation ? m_psgTessellatedShadow : m_psgShadow)->use(m_pDeviceContext);
	if (bToggle) toggleWireframe(); { // Renders solid
		for (UINT i = 0; i < NUM_LIGHTS && i < m_vpLights.size(); i++) {
			if (!m_vpLights[i]->isLit()) continue;
			bLightLit = true;

			m_pDeviceContext->OMSetRenderTargets(1, &m_apRTShadowMaps[i].p, m_pShadowMapDepthStencilView);
			// Clear render target view & depth stencil
			float ClearShadow[4] = { 30.0f, 0.0f, 0.0f, 0.0f }; // RGBA
			m_pDeviceContext->ClearRenderTargetView(m_apRTShadowMaps[i], ClearShadow);
			m_pDeviceContext->ClearDepthStencilView(m_pShadowMapDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

			const Light& l = *m_vpLights[i];
			updateTransformForLight(l);
			renderScene(&bNeedBlur);
		}

		unbindInputBuffers();

		for (UINT i = 0; i < NUM_LIGHTS && i < m_vpLights.size(); i++) {
			if (!m_vpLights[i]->isLit()) continue;

			if (m_bDump) {
				TStringStream tss;
				tss << _T("ShadowMap_") << i + 1;
				dumpIrregularResourceToFile(m_apSRVShadowMaps[i], tss.str(), false,
					XMFLOAT4(1 / 30.0f, 0.0f, 0.0f, 0.0f),
					XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
					XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f),
					DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R16_UNORM);
			}
		}

		if (!bLightLit) {
			// try to determine whether we should blur the irradiance map
			updateTransform();
			bNeedBlur = false;
			for (Renderable* renderable : m_vpRenderables) {
				if (renderable->inScene()) {
					// do frustum culling
					FVector vCenter; float radius;
					renderable->getBoundingSphere(vCenter, radius);
					if (!m_frustum.SphereIntersectsFrustum(XMVectorSet(vCenter.x, vCenter.y, vCenter.z, 1.0f), radius))
						continue;
					bNeedBlur |= (m_bSSS && renderable->supportsSSS() && m_cbSSS.g_sss_strength >= 0.005f);
				}
			}
		}
	} if (bToggle) toggleWireframe();

	m_pDeviceContext->RSSetViewports(1, &m_vpScreen);

	m_rsCurrent = RS_Irradiance;
	renderIrradianceMap(&bNeedBlur);

	if (bToggle) toggleWireframe(); {
		if (bNeedBlur) {
			m_rsCurrent = RS_Gaussian;
			ID3D11ShaderResourceView* apSRVGaussians[NUM_SSS_GAUSSIANS];
			doGaussianBlurs(apSRVGaussians);

			m_rsCurrent = RS_Combine;
			doCombineShading(bNeedBlur, apSRVGaussians);
		}

		if (pSecondaryView) {
			renderViewInView(pSecondaryView);
		}

		if (m_bBloom) {
			m_rsCurrent = RS_Bloom;
			doBloom();
		}
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

	if (m_bDump) {
		// Open the dump folder in explorer
		ShellExecute(nullptr, _T("open"), DEFAULT_DUMP_FOLDER, nullptr, nullptr, SW_SHOWNORMAL);
	}

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
			bNeedBlur |= (m_bSSS && renderable->supportsSSS() && m_cbSSS.g_sss_strength >= 0.005f);
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

void Renderer::doPreRenderings() {
    m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	unbindInputBuffers();

	if (m_bPRAttenuationTexture) {
		// Pre-render attenuation texture
		m_pDeviceContext->RSSetViewports(1, &m_vpAttenuationTexture);
		m_psgSSSAttenuationTexture->use(m_pDeviceContext);
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRTAttenuationTexture.p, nullptr);
		m_pDeviceContext->Draw(6, 0);
		m_nDumpCount = 0;
		TString fileName = _T("atten.png");
		dumpResourceToFile(m_pRTAttenuationTexture, fileName, true, DXGI_FORMAT_R16_UNORM);
		MessageBox(m_hwnd, (_T("Attenuation texture written to ") + fileName).c_str(), _T("Pre-rendering"), MB_OK);
	}
}

void Renderer::bindAttenuationTexture() {
	m_pDeviceContext->PSSetShaderResources(SLOT_ATTENUATION, 1, &m_pSRVAttenuationTexture.p);
	m_pDeviceContext->PSSetSamplers(SLOT_ATTENUATION, 1, &m_pLinearSamplerState.p);
}

void Renderer::unbindAttenuationTexture() {
	ID3D11ShaderResourceView* pNullSRV = nullptr;
	m_pDeviceContext->PSSetShaderResources(SLOT_ATTENUATION, 1, &pNullSRV);
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
	CComPtr<ID3D11RasterizerState> pRS;
	m_pDevice->CreateRasterizerState(&m_descRasterizerState, &pRS);
	m_pDeviceContext->RSSetState(pRS);

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

void Renderer::dumpIrregularResourceToFile(ID3D11ShaderResourceView* pSRV, const TString& strFileName, bool overrideAutoNaming,
										   XMFLOAT4 scaleFactor, XMFLOAT4 defaultValue, XMFLOAT4 lerps,
										   DXGI_FORMAT preferredFormat, DXGI_FORMAT preferredRTFormat)
{
	CComPtr<ID3D11Resource> pResource;
	pSRV->GetResource(&pResource);
	CComQIPtr<ID3D11Texture2D> pT2D = pResource;
	D3D11_TEXTURE2D_DESC desc;
	pT2D->GetDesc(&desc);
	
	DXGI_FORMAT format = (preferredRTFormat != DXGI_FORMAT_UNKNOWN) ? preferredRTFormat : DXGI_FORMAT_R16G16B16A16_UNORM;
	// Create temporary render target view
	CComPtr<ID3D11Texture2D> pTexture2DTarget;
	checkFailure(createTexture2D(m_pDevice, desc.Width, desc.Height, format, &pTexture2DTarget,
		D3D11_BIND_RENDER_TARGET), _T("Failed to create temporary render target texture for irregular resource ") + strFileName);

	CComPtr<ID3D11RenderTargetView> pRTTarget;
	checkFailure(createRTFromTexture2D(m_pDevice, pTexture2DTarget, format, &pRTTarget),
		_T("Failed to create temporary render target view for irregular resource ") + strFileName);

	// Copy render to the temporary RT
	copyRender(pSRV, pRTTarget, false, scaleFactor, defaultValue, lerps);

	// dump the RT texture
	dumpResourceToFile(pTexture2DTarget, strFileName, overrideAutoNaming, preferredFormat);

	// Things should be auto-cleared
}

void Renderer::dumpIrregularResourceToFile(ID3D11RenderTargetView* pRT, const TString& strFileName, bool overrideAutoNaming,
										   XMFLOAT4 scaleFactor, XMFLOAT4 defaultValue, XMFLOAT4 lerps,
										   DXGI_FORMAT preferredFormat, DXGI_FORMAT preferredRTFormat)
{
	// No SRV for copy render here, delegate to texture version
	CComPtr<ID3D11Resource> pResource;
	pRT->GetResource(&pResource);

	dumpIrregularResourceToFile(pResource, strFileName, overrideAutoNaming, scaleFactor, defaultValue, lerps,
		preferredFormat, preferredRTFormat);
}

void Renderer::dumpIrregularResourceToFile(ID3D11Resource* pResource, const TString& strFileName, bool overrideAutoNaming,
										   XMFLOAT4 scaleFactor, XMFLOAT4 defaultValue, XMFLOAT4 lerps,
										   DXGI_FORMAT preferredFormat, DXGI_FORMAT preferredRTFormat)
{
	// Find the DXGI format
	CComQIPtr<ID3D11Texture2D> pT2D = pResource;
	D3D11_TEXTURE2D_DESC desc;
	pT2D->GetDesc(&desc);

	// No SRV for copy render here, create one
	CComPtr<ID3D11ShaderResourceView> pSRV;
	checkFailure(createSRVFromTexture2D(m_pDevice, pT2D, desc.Format, &pSRV),
		_T("Failed to create temporary SRV for irregular resource ") + strFileName);

	// Delegate to SRV version
	dumpIrregularResourceToFile(pSRV, strFileName, overrideAutoNaming, scaleFactor, defaultValue, lerps,
		preferredFormat, preferredRTFormat);
}

void Renderer::dumpResourceToFile(ID3D11ShaderResourceView* pSRV, const TString& strFileName, bool overrideAutoNaming, DXGI_FORMAT preferredFormat) {
	CComPtr<ID3D11Resource> pResource;
	pSRV->GetResource(&pResource);

	dumpResourceToFile(pResource, strFileName, overrideAutoNaming, preferredFormat);
}

void Renderer::dumpResourceToFile(ID3D11RenderTargetView* pRT, const TString& strFileName, bool overrideAutoNaming, DXGI_FORMAT preferredFormat) {
	CComPtr<ID3D11Resource> pResource;
	pRT->GetResource(&pResource);

	dumpResourceToFile(pResource, strFileName, overrideAutoNaming, preferredFormat);
}

void Renderer::dumpResourceToFile(ID3D11Resource* pResource, const TString& strFileName, bool overrideAutoNaming, DXGI_FORMAT preferredFormat) {
	TStringStream tss;
	if (overrideAutoNaming) {
		tss << strFileName;
	} else {
		const TCHAR* szDir = DEFAULT_DUMP_FOLDER;
		if (!CreateDirectory(szDir, nullptr)) {
			if (GetLastError() != ERROR_ALREADY_EXISTS)
				checkFailure(E_FAIL, _T("Failed to create directory for dumping"));
		}
		tss << szDir << "\\" << (++m_nDumpCount) << _T("_") << strFileName << _T(".png");
	}
	SetWindowText(m_hwnd, (_APP_NAME_ _T(" (Capturing ") + tss.str() + _T("...)")).c_str());
	ScratchImage img, newimg;
	CaptureTexture(m_pDevice, m_pDeviceContext, pResource, img);
	const Image* pImage = img.GetImage(0, 0, 0);
	if (preferredFormat != DXGI_FORMAT_UNKNOWN && pImage->format != preferredFormat) {
		// An image format conversion is needed
		checkFailure(Convert(*pImage, preferredFormat, TEX_FILTER_DEFAULT, 0.0f, newimg),
			_T("Failed to convert image to preferred dump format"));
		pImage = newimg.GetImage(0, 0, 0);
	}
	SaveToWICFile(*pImage, WIC_FLAGS_NONE, GUID_ContainerFormatPng, tss.str().c_str());
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
	m_cbMaterial.g_mtBumpMultiplier = m_bBump ? mt.fBumpMultiplier : 0.0f;
	m_cbMaterial.g_mtRoughness = mt.fRoughness;
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
	m_pDeviceContext->PSSetSamplers(SLOT_TEXTURE, 1, &m_pPlaceholderSamplerState.p);
	m_pDeviceContext->PSSetShaderResources(SLOT_TEXTURE, 1, &m_pPlaceholderTexture.p);
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
	m_pDeviceContext->VSSetSamplers(SLOT_BUMPMAP, 1, &m_pBumpSamplerState.p);
	m_pDeviceContext->VSSetShaderResources(SLOT_BUMPMAP, 1, &m_pBumpTexture.p);
	m_pDeviceContext->HSSetSamplers(SLOT_BUMPMAP, 1, &m_pBumpSamplerState.p);
	m_pDeviceContext->HSSetShaderResources(SLOT_BUMPMAP, 1, &m_pBumpTexture.p);
	m_pDeviceContext->DSSetSamplers(SLOT_BUMPMAP, 1, &m_pBumpSamplerState.p);
	m_pDeviceContext->DSSetShaderResources(SLOT_BUMPMAP, 1, &m_pBumpTexture.p);
	m_pDeviceContext->PSSetSamplers(SLOT_BUMPMAP, 1, &m_pBumpSamplerState.p);
	m_pDeviceContext->PSSetShaderResources(SLOT_BUMPMAP, 1, &m_pBumpTexture.p);
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
	m_pDeviceContext->PSSetSamplers(SLOT_NORMALMAP, 1, &m_pBumpSamplerState.p); // use bump map's point sampler
	m_pDeviceContext->PSSetShaderResources(SLOT_NORMALMAP, 1, &m_pNormalTexture.p);
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

void Renderer::updateSSSConstantBufferForParams() {
	XMFLOAT4 tone(0, 0, 0, 1);
	for (UINT i = 0; i < NUM_SSS_GAUSSIANS; i++) {
		XMFLOAT3 coeff = m_sssGaussianParams.coeffs[i];
		tone.x += m_cbSSS.g_sss_coeff_sigma2[i].x = coeff.x;
		tone.y += m_cbSSS.g_sss_coeff_sigma2[i].y = coeff.y;
		tone.z += m_cbSSS.g_sss_coeff_sigma2[i].z = coeff.z;
		float sigma = m_sssGaussianParams.sigmas[i];
		m_cbSSS.g_sss_coeff_sigma2[i].w = sigma * sigma;
	}
	m_cbSSS.g_sss_color_tone = tone;
	// no need to update m_cbSSS, it will be set every render cycle
}

void Renderer::setSkinParams(const VariableParams& vps) {
	m_sssSkinParams = vps;
	m_sssGaussianParams = m_sssGaussianParamsCalculator.getParams(vps);
	for (UINT i = 0; i < NUM_SSS_GAUSSIANS; i++) {
		XMFLOAT3 coeff = m_sssGaussianParams.coeffs[i];
		m_cbCombine.g_weights[i].value = coeff;
	}
	m_pDeviceContext->UpdateSubresource(m_pCombineConstantBuffer, 0, NULL, &m_cbCombine, 0, 0);
	updateSSSConstantBufferForParams();
}
