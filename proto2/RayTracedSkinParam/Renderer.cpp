/**
 * Main renderer class
 */

#include "stdafx.h"

#include "Renderer.h"

using namespace RLSkin;
using namespace Utils;

Renderer::Renderer(HWND hwnd, CRect rectView)
	: m_hwnd(hwnd), m_rectView(rectView)
{
	m_context = OpenRLCreateContext(nullptr, (OpenRLNotify)onError, this);
	OpenRLSetCurrentContext(m_context);
}

Renderer::~Renderer() {
	OpenRLDestroyContext(m_context);
}

void Renderer::render() {

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
