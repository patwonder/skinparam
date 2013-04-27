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
		D3D11_RASTERIZER_DESC m_descRasterizerState;

		std::vector<IUnknown**> m_vppCOMObjs;

		// 3D Transformation
		struct TransformConstantBuffer {
			XMFLOAT4X4 g_matWorld;
			XMFLOAT4X4 g_matViewProj;
			XMFLOAT3 g_posEye;
			float pad;
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
			float pad3;
			XMFLOAT3 g_mtEmissive;
			float g_mtShininess;
		};
		ID3D11Buffer* m_pTransformConstantBuffer;
		TransformConstantBuffer m_cbTransform;
		ID3D11Buffer* m_pLightingConstantBuffer;
		LightingConstantBuffer m_cbLighting;
		ID3D11Buffer* m_pMaterialConstantBuffer;
		MaterialConstantBuffer m_cbMaterial;

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

		// IRenderer implementation
		void setWorldMatrix(const XMMATRIX& matWorld) override;
		void setMaterial(const Material& material) override;
		void usePlaceholderTexture() override;
	};

} // namespace Skin
