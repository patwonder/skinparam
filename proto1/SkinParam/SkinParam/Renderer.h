/**
 * Main renderer class
 */

#pragma once

#include <vector>

#include "Renderable.h"

namespace Skin {
	class Config;

	class Renderer {
	private:
		// Parameters
		HWND m_hwnd;
		CRect m_rectView;
		Config* m_pConfig;

		// D3D stuff
		D3D_DRIVER_TYPE m_driverType;
		D3D_FEATURE_LEVEL m_featureLevel;

		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		IDXGISwapChain* m_pSwapChain;
		ID3D11RenderTargetView* m_pRenderTargetView;

		std::vector<IUnknown**> m_vppCOMObjs;

		// Rendering
		Camera* m_pCamera;
		std::vector<Renderable*> m_vpRenderables;

		// Statistics
		unsigned int m_nFrameCount;
		DWORD m_nStartTick;
		float m_fps;

		HRESULT initDX();
		HRESULT initStuff();

		void computeStats();
		void renderScene();

		static HRESULT compileShader(const Utils::TString& strFileName, const char* szEntryPoint, const char* szShaderModel, ID3DBlob** ppBlobOut);
		static void checkFailure(HRESULT hr, const Utils::TString& prompt);
	public:
		Renderer(HWND hwnd, CRect rectView, Config* pConfig, Camera* pCamera);
		~Renderer();

		void addRenderable(Renderable* renderable);
		void removeRenderable(Renderable* renderable);
		void removeAllRenderables();

		void render();

		// Statistics
		float getFPS() const { return m_fps; }
	};

} // namespace Skin
