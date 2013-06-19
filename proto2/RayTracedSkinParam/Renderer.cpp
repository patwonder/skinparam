/**
 * Main renderer class
 */

#include "stdafx.h"

#include "Renderer.h"

using namespace RLSkin;

Renderer::Renderer(HWND hwnd, CRect rectView)
	: m_hwnd(hwnd), m_rectView(rectView)
{
	m_context = OpenRLCreateContext(nullptr, (OpenRLNotify)nullptr, nullptr);
	OpenRLSetCurrentContext(m_context);
}

Renderer::~Renderer() {
	OpenRLDestroyContext(m_context);
}

void Renderer::render() {

}
