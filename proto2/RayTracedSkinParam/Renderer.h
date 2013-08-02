/**
 * Main renderer class
 */

#pragma once

#include "Program.h"
#include "RLPtr.h"
#include "Drawable.h"

namespace RLSkin {
	class ShaderGroup;
	class Renderer {
	private:
		// constructor parameters
		HWND m_hwnd;
		CRect m_rectView;

		// Drawables management
		std::vector<Drawable*> m_vpDrawables;

		// OpenRL context
		OpenRLContext m_rlContext;
		RLtexture m_rlMainTexture;
		RLframebuffer m_rlMainFramebuffer;
		RLbuffer m_rlTempBuffer;

		// Shaders
		RLPtr<Shader> m_pvsdDrawable;
		RLPtr<Shader> m_prsdDrawable;
		RLPtr<Program> m_pprgDrawable;
		RLPtr<Shader> m_pfsdMain;
		RLPtr<Program> m_pprgMain;

		// Direct3D backend
		D3D_DRIVER_TYPE m_d3dDriverType;
		D3D_FEATURE_LEVEL m_d3dFeatureLevel;
		CComPtr<ID3D11Device> m_pd3dDevice;
		CComPtr<ID3D11DeviceContext> m_pd3dDeviceContext;
		CComPtr<IDXGISwapChain> m_pd3dSwapChain;
		CComPtr<ID3D11RenderTargetView> m_pd3dRenderTargetView;
		D3D11_VIEWPORT m_d3dScreenViewport;

		ShaderGroup* m_psgDirectDraw;
		CComPtr<ID3D11ShaderResourceView> m_pSRVResult;
		CComPtr<ID3D11SamplerState> m_pd3dLinearSampler;
		CComPtr<ID3D11SamplerState> m_pd3dPointSampler;

		void initRL();
		void initShaders();
		void uninitShaders();

		HRESULT initDX();
		void initDXMiscellaneous();
		void initDXShaders();
		void uninitDXShaders();
		void uninitDX();

		void setupProjection();
	public:
		Renderer(HWND hwnd, CRect rectView);
		~Renderer();

		void addDrawable(Drawable* pDrawable);
		void removeDrawable(Drawable* pDrawable);
		void removeAllDrawables();

		void render();
		void onError(RLenum error, const void* privateData, size_t privateSize, const char* message);
		static void onError(RLenum error, const void* privateData, size_t privateSize, const char* message, void* userData);
	};
} // namespace RLSkin
