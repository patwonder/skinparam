/**
 * Main renderer class
 */

#pragma once

#include "Program.h"
#include "Primitive.h"
#include "RLPtr.h"
#include "Drawable.h"
#include "Camera.h"

namespace RLSkin {
	class ShaderGroup;
	class Renderer {
	private:
		// constructor parameters
		HWND m_hwnd;
		CRect m_rectView;
		Camera* m_pCamera;

		// Drawables management
		std::vector<Drawable*> m_vpDrawables;

		// OpenRL context
		OpenRLContext m_rlContext;
		RLtexture m_rlMainTexture;
		RLframebuffer m_rlMainFramebuffer;
		RLbuffer m_rlTempBuffer;

		// Shaders
		RLPtr<Program> m_pprgDrawable;
		RLPtr<Program> m_pprgMain;
		RLPtr<Program> m_pprgLight;
		RLPtr<Program> m_pprgEnvironment;

		// Uniform blocks
		RLPtr<Buffer> m_pLightBuffer;
		struct UB_Light {
			XMFLOAT4 position_radius;
			RLprimitive prim;
		} m_ubLight;

		// Primitives
		RLPtr<Primitive> m_pLightPrimitive;
		RLPtr<Primitive> m_pEnvironmentPrimitive;

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
		void initLight();
		void updateLight();
		void drawLight();

		HRESULT initDX();
		void initDXMiscellaneous();
		void initDXShaders();
		void uninitDXShaders();
		void uninitDX();

		void setupProjection();
	public:
		Renderer(HWND hwnd, CRect rectView, Camera* pCamera);
		~Renderer();

		void addDrawable(Drawable* pDrawable);
		void removeDrawable(Drawable* pDrawable);
		void removeAllDrawables();

		void render();
		void onError(RLenum error, const void* privateData, size_t privateSize, const char* message);
		static void onError(RLenum error, const void* privateData, size_t privateSize, const char* message, void* userData);
	};
} // namespace RLSkin
