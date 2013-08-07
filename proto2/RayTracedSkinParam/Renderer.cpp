/**
 * Main renderer class
 */

#include "stdafx.h"

#include "Renderer.h"
#include "D3DHelper.h"
#include "ShaderGroup.h"

using namespace RLSkin;
using namespace Utils;

Renderer::Renderer(HWND hwnd, CRect rectView, Camera* pCamera)
	: m_hwnd(hwnd), m_rectView(rectView), m_pCamera(pCamera),
	  m_psgDirectDraw(nullptr)
{
	initRL();
	initShaders();
	initLight();

	D3DHelper::checkFailure(initDX(), _T("Failed to initialize Direct3D"));
	initDXMiscellaneous();
	initDXShaders();
}

Renderer::~Renderer() {
	uninitDXShaders();
	uninitDX();

	uninitShaders();

	rlDeleteBuffers(1, &m_rlTempBuffer);
	rlDeleteFramebuffers(1, &m_rlMainFramebuffer);
	rlDeleteTextures(1, &m_rlMainTexture);
	// Not friendly in vs2012 debugger
	//OpenRLDestroyContext(m_rlContext);
}

void Renderer::onError(RLenum error, const void* privateData, size_t privateSize, const char* message) {
	TString strMessage = Utils::TStringFromANSIString(message);
	TRACE(_T("[RLSkin ERROR %d] %s"), error, strMessage.c_str());

	TStringStream tss;
	tss << _T("ERROR ") << error << _T(": ") << strMessage;
	MessageBox(m_hwnd, tss.str().c_str(), APP_NAME _T(" ERROR"), MB_OK | MB_ICONERROR);
	exit(EXIT_FAILURE);
}

void Renderer::onError(RLenum error, const void* privateData, size_t privateSize, const char* message, void* userData) {
	// dispatch to the Renderer instance specified by userData
	static_cast<Renderer*>(userData)->onError(error, privateData, privateSize, message);
}

void Renderer::initRL() {
	// Create OpenRL context
	OpenRLContextAttribute attributes[] = {
//		kOpenRL_RequireHardwareAcceleration,
		kOpenRL_ExcludeCPUCores, 0,
		kOpenRL_DisableHyperthreading, 0,
		kOpenRL_DisableStats, 0,
		kOpenRL_DisableProfiling, 0,
		kOpenRL_DisableTokenStream, 0,
		NULL
	};
	
	m_rlContext = OpenRLCreateContext(attributes, (OpenRLNotify)onError, this);
	OpenRLSetCurrentContext(m_rlContext);

	// Create the framebuffer texture
	rlGenTextures(1, &m_rlMainTexture);
	rlBindTexture(RL_TEXTURE_2D, m_rlMainTexture);
	rlTexImage2D(RL_TEXTURE_2D, 0, RL_RGBA, m_rectView.Width(), m_rectView.Height(), 0, RL_RGBA, RL_FLOAT, NULL);

	// Create the framebuffer object to render to
	// and attach the texture that will store the rendered image.
	rlGenFramebuffers(1, &m_rlMainFramebuffer);
	rlBindFramebuffer(RL_FRAMEBUFFER, m_rlMainFramebuffer);
	rlFramebufferTexture2D(RL_FRAMEBUFFER, RL_COLOR_ATTACHMENT0, RL_TEXTURE_2D, m_rlMainTexture, 0);

	// Setup the viewport
	rlViewport(0, 0, m_rectView.Width(), m_rectView.Height());

	// Create the buffer to copy the rendered image into
	rlGenBuffers(1, &m_rlTempBuffer);
	rlBindBuffer(RL_PIXEL_PACK_BUFFER, m_rlTempBuffer);
	rlBufferData(RL_PIXEL_PACK_BUFFER,
				 m_rectView.Width() * m_rectView.Height() * 4 * sizeof(float),
				 0,
				 RL_STATIC_DRAW);
}

void Renderer::initShaders() {
	RLPtr<Shader> pfsdMain, prsdDrawable, pvsdDrawable, pvsdDefault, prsdLight, prsdEnvironment;
	Shader::createInstance(_T("Shaders/frame.rlsl.glsl"), RL_FRAME_SHADER, &pfsdMain);
	Program::createInstance(std::vector<Shader*>(&pfsdMain.p, &pfsdMain.p + 1), &m_pprgMain);

	Shader::createInstance(_T("Shaders/ray.rlsl.glsl"), RL_RAY_SHADER, &prsdDrawable);
	Shader::createInstance(_T("Shaders/vertex.rlsl.glsl"), RL_VERTEX_SHADER, &pvsdDrawable);
	Shader* apDrawableShaders[] = {
		prsdDrawable.p,
		pvsdDrawable.p
	};
	Program::createInstance(std::vector<Shader*>(apDrawableShaders, apDrawableShaders + ARRAYSIZE(apDrawableShaders)),
		&m_pprgDrawable);

	Shader::createInstance(_T("Shaders/default.vertex.rlsl.glsl"), RL_VERTEX_SHADER, &pvsdDefault);

	Shader::createInstance(_T("Shaders/light.ray.rlsl.glsl"), RL_RAY_SHADER, &prsdLight);
	Shader* apLightShaders[] = {
		prsdLight.p,
		pvsdDefault.p
	};
	Program::createInstance(std::vector<Shader*>(apLightShaders, apLightShaders + ARRAYSIZE(apLightShaders)), &m_pprgLight);

	Shader::createInstance(_T("Shaders/env.ray.rlsl.glsl"), RL_RAY_SHADER, &prsdEnvironment);
	Shader* apEnvironmentShaders[] = {
		prsdEnvironment.p,
		pvsdDefault.p
	};
	Program::createInstance(std::vector<Shader*>(apEnvironmentShaders, apEnvironmentShaders + ARRAYSIZE(apEnvironmentShaders)),
		&m_pprgEnvironment);
}

void Renderer::uninitShaders() {
	m_pprgMain.Release();
	m_pprgDrawable.Release();
}

void Renderer::addDrawable(Drawable* pDrawable) {
	pDrawable->init();
	m_vpDrawables.push_back(pDrawable);
}

void Renderer::removeDrawable(Drawable* pDrawable) {
	for (size_t i = 0; i < m_vpDrawables.size(); i++) {
		if (pDrawable == m_vpDrawables[i]) {
			m_vpDrawables.erase(m_vpDrawables.begin() + i);
			pDrawable->cleanup();
			break;
		}
	}
}

void Renderer::removeAllDrawables() {
	m_vpDrawables.clear();
}

void Renderer::setupProjection() {
	Vector vecEye, vecLookAt, vecUp;
	m_pCamera->look(vecEye, vecLookAt, vecUp);
	XMMATRIX matView = XMMatrixLookAtLH(XMVectorSet((float)vecEye.x, (float)vecEye.y, (float)vecEye.z, 1.0f),
										XMVectorSet((float)vecLookAt.x, (float)vecLookAt.y, (float)vecLookAt.z, 1.0f),
										XMVectorSet((float)vecUp.x, (float)vecUp.y, (float)vecUp.z, 1.0f));
	XMMATRIX matProj = XMMatrixPerspectiveFovLH((float)(Math::PI / 6), (float)m_rectView.Width() / m_rectView.Height(), 0.1f, 20.0f);
	XMMATRIX matViewProj = matView * matProj;
	XMVECTOR vecDeterminant;
	XMMATRIX matInvViewProj = XMMatrixInverse(&vecDeterminant, matViewProj);
	XMFLOAT4X4 matStoredIVP;
	XMStoreFloat4x4(&matStoredIVP, matInvViewProj);
	RLint locViewProjMatrix = m_pprgMain->getUniformLocation("g_matInvViewProj");
	rlUniformMatrix4fv(locViewProjMatrix, 1, RL_FALSE, (RLfloat*)&matStoredIVP);
}

void Renderer::initLight() {
	Primitive::createInstance(&m_pLightPrimitive);
	Primitive::createInstance(&m_pEnvironmentPrimitive);
	m_ubLight.prim = m_pLightPrimitive->getRLHandle();
	RLHelper::createUniformBuffer(m_ubLight, &m_pLightBuffer);
}

void Renderer::updateLight() {
	m_ubLight.position_radius = XMFLOAT4(6.0f, -4.0f, 2.0f, 3.0f);
	RLHelper::updateUniformBuffer(m_pLightBuffer, m_ubLight);
}

void Renderer::drawLight() {
	m_pLightPrimitive->useProgram(m_pprgLight);
	RLint locColor = m_pprgLight->getUniformLocation("g_color");
	rlUniform3f(locColor, 1.0f, 1.0f, 1.0f);

	m_pEnvironmentPrimitive->useProgram(m_pprgEnvironment);
	locColor = m_pprgEnvironment->getUniformLocation("g_color");
	rlUniform3f(locColor, 0.01f, 0.01f, 0.01f);
}

void Renderer::render() {
	rlClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	rlClear(RL_COLOR_BUFFER_BIT);

	updateLight();
	drawLight();

	for (Drawable* pDrawable : m_vpDrawables) {
		pDrawable->draw(m_pprgDrawable, m_pLightBuffer, m_pEnvironmentPrimitive);
	}

	rlBindPrimitive(RL_PRIMITIVE, RL_NULL_PRIMITIVE);
	m_pprgMain->use();

	setupProjection();

	rlRenderFrame();

	rlBindBuffer(RL_PIXEL_PACK_BUFFER, m_rlTempBuffer);
	rlBindTexture(RL_TEXTURE_2D, m_rlMainTexture);
	rlGetTexImage(RL_TEXTURE_2D, 0, RL_RGBA, RL_FLOAT, NULL);
	float* pixels = (float*)rlMapBuffer(RL_PIXEL_PACK_BUFFER, RL_READ_ONLY);

	// Here is where you copy the data out.
	CComPtr<ID3D11Resource> pResource;
	m_pSRVResult->GetResource(&pResource);
	D3D11_MAPPED_SUBRESOURCE mapped;
	D3DHelper::checkFailure(m_pd3dDeviceContext->Map(pResource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped),
		_T("Failed to map texture for write"));
	// Copy one row at a time
	for (int i = 0; i < m_rectView.Height(); i++)
		memcpy((void*)((ULONG_PTR)mapped.pData + mapped.RowPitch * i), pixels + m_rectView.Width() * 4 * i,
			m_rectView.Width() * sizeof(float) * 4);
	m_pd3dDeviceContext->Unmap(pResource, 0);

	// We no longer need access to the original pixels
	rlUnmapBuffer(RL_PIXEL_PACK_BUFFER);
	rlBindBuffer(RL_PIXEL_PACK_BUFFER, NULL);
	
	// Render using d3d backend
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_pd3dDeviceContext->ClearRenderTargetView(m_pd3dRenderTargetView, clearColor);
	m_pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT zero = 0;
	ID3D11Buffer* nullBuffer = nullptr;
	m_pd3dDeviceContext->IASetVertexBuffers(0, 1, &nullBuffer, &zero, &zero);
	m_pd3dDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

	m_psgDirectDraw->use(m_pd3dDeviceContext);
	m_pd3dDeviceContext->PSSetShaderResources(0, 1, &m_pSRVResult.p);
	ID3D11SamplerState* samplers[] = { m_pd3dLinearSampler.p, m_pd3dPointSampler.p };
	m_pd3dDeviceContext->PSSetSamplers(0, 2, samplers);

	m_pd3dDeviceContext->Draw(6, 0);

	ID3D11ShaderResourceView* pNullSRV = nullptr;
	m_pd3dDeviceContext->PSSetShaderResources(0, 1, &pNullSRV);

	m_pd3dSwapChain->Present(0, 0);
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
		D3D_FEATURE_LEVEL_10_0
    };
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
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

	HRESULT hr;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
		m_d3dDriverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, m_d3dDriverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &m_pd3dSwapChain, &m_pd3dDevice, &m_d3dFeatureLevel, &m_pd3dDeviceContext);
        if (SUCCEEDED(hr))
            break;
    }
    if (FAILED(hr))
        return hr;

    // Create a render target view
    CComPtr<ID3D11Texture2D> pBackBuffer;
	hr = m_pd3dSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return hr;

	hr = m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pd3dRenderTargetView);
	if (FAILED(hr))
		return hr;

	m_pd3dDeviceContext->OMSetRenderTargets(1, &m_pd3dRenderTargetView.p, nullptr);

	// Setup the viewport
    D3D11_VIEWPORT& vp = m_d3dScreenViewport;
	vp.Width = (float)m_rectView.Width();
	vp.Height = (float)m_rectView.Height();
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
	vp.TopLeftX = (float)m_rectView.left;
	vp.TopLeftY = (float)m_rectView.top;
    m_pd3dDeviceContext->RSSetViewports(1, &vp);

	return S_OK;
}

void Renderer::initDXMiscellaneous() {
	using namespace D3DHelper;
	// Temporary texture for drawing the result
	CComPtr<ID3D11Texture2D> pTexture2D;
	checkFailure(createDynamicTexture2D(m_pd3dDevice, m_rectView.Width(), m_rectView.Height(), DXGI_FORMAT_R32G32B32A32_FLOAT,
		&pTexture2D, D3D11_BIND_SHADER_RESOURCE), _T("Failed to create dynamic texture for OpenRL result"));
	checkFailure(createSRVFromTexture2D(m_pd3dDevice, pTexture2D, DXGI_FORMAT_R32G32B32A32_FLOAT, &m_pSRVResult),
		_T("Failed to create SRV for OpenRL result"));

	checkFailure(createSamplerState(m_pd3dDevice, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, &m_pd3dLinearSampler),
		_T("Failed to create linear sampler state"));

	checkFailure(createSamplerState(m_pd3dDevice, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, &m_pd3dPointSampler),
		_T("Failed to create point sampler state"));
}

void Renderer::initDXShaders() {
	// Screen-space shaders
	D3D11_INPUT_ELEMENT_DESC empty_layout[] = { { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 } };
	m_psgDirectDraw = new ShaderGroup(m_pd3dDevice, _T("DirectDraw.fx"), empty_layout, 0,
		"VS_Quad", nullptr, nullptr, "PS_Point_UpsideDown");
}

void Renderer::uninitDXShaders() {
	delete m_psgDirectDraw;
	m_psgDirectDraw = nullptr;
}

void Renderer::uninitDX() {
	if (m_pd3dDeviceContext)
		m_pd3dDeviceContext->ClearState();
}
