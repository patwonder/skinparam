/**
 * Main renderer class
 */

#pragma once

#include <vector>

#include "Renderable.h"
#include "Color.h"

namespace Skin {
	class Config;
	class ShaderGroup;
	struct Light;

	class Renderer : public IRenderer {
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
		ID3D11Texture2D* m_pDepthStencil;
		ID3D11DepthStencilView* m_pDepthStencilView;
		ID3D11SamplerState* m_pPlaceholderSamplerState;
		ID3D11ShaderResourceView* m_pPlaceholderTexture;
		ID3D11SamplerState* m_pBumpSamplerState;
		ID3D11ShaderResourceView* m_pBumpTexture;
		D3D11_RASTERIZER_DESC m_descRasterizerState;

		std::vector<IUnknown**> m_vppCOMObjs;

		static const UINT SLOT_TEXTURE = 0;
		static const UINT SLOT_BUMPMAP = 1;

		// 3D Transformation
		struct TransformConstantBuffer {
			XMFLOAT4X4 g_matWorld;
			XMFLOAT4X4 g_matViewProj;
			XMFLOAT3 g_posEye;
			float g_fAspectRatio;
			XMFLOAT4 g_vFrustrumPlaneEquation[4];
		};
		struct RLight {
			XMFLOAT3 ambient;
			float pad1;
			XMFLOAT3 diffuse;
			float pad2;
			XMFLOAT3 specular;
			float pad3;
			XMFLOAT3 attenuation;
			float pad4;
			XMFLOAT3 position;
			float pad5;
		};
		struct LightingConstantBuffer {
			static const UINT NUM_LIGHTS = 2;
			RLight g_lights[NUM_LIGHTS];
			XMFLOAT3 g_ambient;
			float pad;
		};
		struct MaterialConstantBuffer {
			XMFLOAT3 g_mtAmbient;
			float pad1;
			XMFLOAT3 g_mtDiffuse;
			float pad2;
			XMFLOAT3 g_mtSpecular;
			float g_mtShininess;
			XMFLOAT3 g_mtEmissive;
			float g_mtBumpMultiplier;
		};
		struct TessellationConstantBuffer {
			XMFLOAT4 g_vTesselationFactor;
		};
		ID3D11Buffer* m_pTransformConstantBuffer;
		TransformConstantBuffer m_cbTransform;
		ID3D11Buffer* m_pLightingConstantBuffer;
		LightingConstantBuffer m_cbLighting;
		ID3D11Buffer* m_pMaterialConstantBuffer;
		MaterialConstantBuffer m_cbMaterial;
		ID3D11Buffer* m_pTessellationConstantBuffer;
		TessellationConstantBuffer m_cbTessellation;

		XMFLOAT4X4 m_matProjection; // use XMFLOAT4X4 instead of XMMATRIX to resolve alignment issues

		// Rendering
		Camera* m_pCamera;
		std::vector<Renderable*> m_vpRenderables;
		std::vector<Light*> m_vpLights;

		ShaderGroup* m_psgTriangle;
		ShaderGroup* m_psgPhong;
		ShaderGroup* m_psgTesselatedPhong;

		// Statistics
		unsigned int m_nFrameCount;
		DWORD m_nStartTick;
		float m_fps;

		// toggleables
		bool m_bTessellation;
		bool m_bBump;

		HRESULT initDX();
		void initMisc();

		void loadShaders();
		void unloadShaders();

		void initTransform();
		void updateTransform();
		void initLighting();
		void updateLighting();
		void initMaterial();
		void updateProjection();
		void initTessellation();
		void updateTessellation();

		void setConstantBuffers();
		void computeStats();
		void renderScene();
	public:
		Renderer(HWND hwnd, CRect rectView, Config* pConfig, Camera* pCamera);
		~Renderer();

		void addRenderable(Renderable* renderable);
		void removeRenderable(Renderable* renderable);
		void removeAllRenderables();

		void setGlobalAmbient(const Utils::Color& coAmbient);
		void addLight(Light* light);
		void removeLight(Light* light);
		void removeAllLights();

		void render();

		// Statistics
		float getFPS() const { return m_fps; }

		// Toggleables
		void toggleWireframe();
		void toggleTessellation();
		void toggleBump();

		// IRenderer implementation
		void setWorldMatrix(const XMMATRIX& matWorld) override;
		void setMaterial(const Material& material) override;
		void useTexture(ID3D11SamplerState* pTextureSamplerState, ID3D11ShaderResourceView* pTexture) override;
		void usePlaceholderTexture() override;
		void useBumpMap(ID3D11SamplerState* pBumpMapSamplerState, ID3D11ShaderResourceView* pBumpMap) override;
		void usePlaceholderBumpMap() override;
		void setTessellationFactor(float edge, float inside, float min, float desiredSize) override;
	};

} // namespace Skin
