/**
 * Main renderer class
 */

#include "stdafx.h"

#include "Renderer.h"

using namespace RLSkin;
using namespace Utils;

Renderer::Renderer(HWND hwnd, CRect rectView)
	: m_hwnd(hwnd), m_rectView(rectView),
	  m_pMainFrameShader(nullptr),
	  m_pMainProgram(nullptr)
{
	initRL();
	initShaders();
}

Renderer::~Renderer() {
	uninitShaders();

	rlDeleteBuffers(1, &m_rlTempBuffer);
	rlDeleteFramebuffers(1, &m_rlMainFramebuffer);
	rlDeleteTextures(1, &m_rlMainTexture);
	OpenRLDestroyContext(m_rlContext);
}

void Renderer::onError(RLenum error, const void* privateData, size_t privateSize, const char* message) {
	TString strMessage = Utils::TStringFromANSIString(message);
	TRACE(_T("[RLSkin ERROR %d] %s"), error, strMessage.c_str());

	TStringStream tss;
	tss << _T("ERROR ") << error << _T(": ") << strMessage;
	MessageBox(m_hwnd, tss.str().c_str(), APP_NAME _T(" ERROR"), MB_OK | MB_ICONERROR);
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
	m_vppShaders.push_back(&m_pMainFrameShader);
	m_vppPrograms.push_back(&m_pMainProgram);

	m_pMainFrameShader = new FrameShader(_T("Shaders/frame.rlsl.glsl"));
	m_pMainProgram = new Program(std::vector<Shader*>(&m_pMainFrameShader, &m_pMainFrameShader + 1));
}

void Renderer::uninitShaders() {
	for (Program** ppProgram : m_vppPrograms) {
		delete *ppProgram;
		*ppProgram = nullptr;
	}
	m_vppPrograms.clear();
	for (Shader** ppShader : m_vppShaders) {
		delete *ppShader;
		*ppShader = nullptr;
	}
	m_vppShaders.clear();
}

void Renderer::render() {
	rlBindPrimitive(RL_PRIMITIVE, RL_NULL_PRIMITIVE);
	m_pMainProgram->use();
	rlRenderFrame();

	rlBindBuffer(RL_PIXEL_PACK_BUFFER, m_rlTempBuffer);
	rlBindTexture(RL_TEXTURE_2D, m_rlMainTexture);
	rlGetTexImage(RL_TEXTURE_2D, 0, RL_RGBA, RL_FLOAT, NULL);
	float* pixels = (float*)rlMapBuffer(RL_PIXEL_PACK_BUFFER, RL_READ_ONLY);

	// Here is where you copy the data out.


	// We no longer need access to the original pixels
	rlUnmapBuffer(RL_PIXEL_PACK_BUFFER);
	rlBindBuffer(RL_PIXEL_PACK_BUFFER, NULL);
}
