/**
 * Main renderer class
 */

#pragma once

#include <vector>

#include "Renderable.h"
#include "RenderableManager.h"
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
		RenderableManager* m_pRenderableManager;

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
		ID3D11ShaderResourceView* m_pNormalTexture;
		ID3D11SamplerState* m_pLinearSamplerState;
		D3D11_RASTERIZER_DESC m_descRasterizerState;
		D3D11_VIEWPORT m_vpScreen;

		// Subsurface scattering
		static const UINT NUM_SSS_GAUSSIANS = 6;
		static const UINT IDX_SSS_IRRADIANCE = 0;
		static const UINT IDX_SSS_ALBEDO = 1;
		static const UINT IDX_SSS_SPECULAR = 2;
		static const UINT IDX_SSS_DIFFUSE_STENCIL = 3;
		static const UINT IDX_SSS_GAUSSIANS_START = 4;
		static const UINT IDX_SSS_TEMPORARY = IDX_SSS_GAUSSIANS_START + NUM_SSS_GAUSSIANS;
		// irradiance, albedo, specular, diffuse stencil, gaussians and a temporary view
		static const UINT NUM_SSS_VIEWS = IDX_SSS_TEMPORARY + 1;
		ID3D11RenderTargetView* m_apRTSSS[NUM_SSS_VIEWS];
		ID3D11ShaderResourceView* m_apSRVSSS[NUM_SSS_VIEWS];
		ShaderGroup* m_psgSSSIrradiance;
		ShaderGroup* m_psgSSSIrradianceNoTessellation;
		ShaderGroup* m_psgSSSGausianVertical;
		ShaderGroup* m_psgSSSGausianHorizontal;
		ShaderGroup* m_psgSSSCombine;
		ShaderGroup* m_psgSSSCombineAA;
		static const float SSS_GAUSSIAN_KERNEL_SIGMA[NUM_SSS_GAUSSIANS];
		static const float SSS_GAUSSIAN_WEIGHTS[NUM_SSS_GAUSSIANS][3];

		struct GaussianConstantBuffer {
			float g_blurWidth; // blur width
			float g_invAspectRatio; // inverse of aspect ratio (height / width)
			float pad[2];
		};
		struct CombineConstantBuffer {
			struct {
				XMFLOAT3 value;
				float pad;
			} g_weights[NUM_SSS_GAUSSIANS];
		};
		struct SSSConstantBuffer {
			float g_sss_intensity;
			float pad[3];
		};

		ID3D11Buffer* m_pGaussianConstantBuffer;
		GaussianConstantBuffer m_cbGaussian;
		ID3D11Buffer* m_pCombineConstantBuffer;
		CombineConstantBuffer m_cbCombine;
		ID3D11Buffer* m_pSSSConstantBuffer;
		SSSConstantBuffer m_cbSSS;

		// Post-process Anti-aliasing
		ShaderGroup* m_psgPostProcessAA;

		struct PostProcessConstantBuffer {
			XMFLOAT2 rcpFrame;
			float pad[2];
			XMFLOAT4 rcpFrameOpt;
		};
		ID3D11Buffer* m_pPostProcessConstantBuffer;
		PostProcessConstantBuffer m_cbPostProcess;

		std::vector<IUnknown**> m_vppCOMObjs;
		std::vector<ShaderGroup**> m_vppShaderGroups;

		static const UINT SLOT_TEXTURE = 0;
		static const UINT SLOT_BUMPMAP = 1;
		static const UINT SLOT_NORMALMAP = 2;
		static const UINT SLOT_SHADOWMAP = 3;

		static const UINT NUM_LIGHTS = 2;
		// 3D Transformation
		struct TransformConstantBuffer {
			XMFLOAT4X4 g_matWorld;
			XMFLOAT4X4 g_matView;
			XMFLOAT4X4 g_matViewProj;
			XMFLOAT4X4 g_matViewLights[NUM_LIGHTS];
			XMFLOAT4X4 g_matViewProjLights[NUM_LIGHTS];
			XMFLOAT4X4 g_matViewProjCamera;
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
		XMFLOAT4X4 m_matLightProjection; // light projection matrix

		// Shadow mapping
		static const UINT SM_SIZE = 1024;
		static const UINT IDX_SHADOW_TEMPORARY = NUM_LIGHTS;
		static const UINT NUM_SHADOW_VIEWS = IDX_SHADOW_TEMPORARY + 1;
		D3D11_VIEWPORT m_vpShadowMap;
		ID3D11RenderTargetView* m_apRTShadowMaps[NUM_SHADOW_VIEWS];
		ID3D11ShaderResourceView* m_apSRVShadowMaps[NUM_SHADOW_VIEWS];
		ID3D11DepthStencilView* m_pShadowMapDepthStencilView;
		ID3D11SamplerState* m_pShadowMapSamplerState;
		ShaderGroup* m_psgGaussianShadowVertical;
		ShaderGroup* m_psgGaussianShadowHorizontal;

		// constant buffer for shadow map blurring
		struct GaussianShadowConstantBuffer {
			XMFLOAT2 rcpScreenSize;
			float pad[2];
		};
		ID3D11Buffer* m_pGaussianShadowConstantBuffer;
		GaussianShadowConstantBuffer m_cbGaussainShadow;

		// Rendering
		Camera* m_pCamera;
		std::vector<Renderable*> m_vpRenderables;
		std::vector<Light*> m_vpLights;

		ShaderGroup* m_psgShadow;
		ShaderGroup* m_psgTessellatedPhong;
		ShaderGroup* m_psgTessellatedShadow;

		// Statistics
		unsigned int m_nFrameCount;
		DWORD m_nStartTick;
		float m_fps;

		// toggleables
		bool m_bTessellation;
		bool m_bBump;
		bool m_bSSS;
		bool m_bPostProcessAA;
		bool m_bVSMBlur;
		bool m_bDump;
		UINT m_nDumpCount;

		// render stage control
		enum RenderStage {
			NotRendering,
			ShadowMap,
			Irradiance,
			Gaussian,
			Combine,
			RS_PostProcessAA,
			UI
		};
		RenderStage m_rsCurrent;

		HRESULT initDX();
		void initMisc();

		void loadShaders();
		void unloadShaders();

		void initTransform();
		XMMATRIX getViewProjMatrix(const Camera& camera, const XMMATRIX& matProjection, XMMATRIX& matView);
		Camera getLightCamera(const Light& light);
		void updateTransform();
		void updateTransformForLight(const Light& light);
		void updateTransform(const Camera& camera, const XMMATRIX& matProjection, float fAspectRatio);
		void initLighting();
		void updateLighting();
		void initMaterial();
		void updateProjection();
		void initTessellation();
		void updateTessellation();
		void initShadowMaps();
		void bindShadowMaps();
		void unbindShadowMaps();

		void initSSS();
		void renderIrradianceMap(ID3D11RenderTargetView* pRTIrradiance, ID3D11RenderTargetView* pRTAlbedo,
			ID3D11RenderTargetView* pRTDiffuseStencil, ID3D11RenderTargetView* pRTSpecular, bool* opbNeedBlur = nullptr);
		void setGaussianKernelSize(float size);
		void unbindInputBuffers();
		void bindGaussianConstantBuffer();
		void bindCombineConstantBuffer();
		void doGaussianBlurs();
		void doCombineShading(bool bNeedBlur);

		void initPostProcessAA();
		void bindPostProcessConstantBuffer();
		void doPostProcessAA();

		void blurShadowMaps();

		void dumpShaderResourceViewToFile(ID3D11ShaderResourceView* pSRV, const Utils::TString& strFileName);
		void dumpRenderTargetToFile(ID3D11RenderTargetView* pRT, const Utils::TString& strFileName);
		void dumpTextureToFile(ID3D11Resource* pTexture2D, const Utils::TString& strFileName);

		void setConstantBuffers();
		void computeStats();
		void renderScene(bool* opbNeedBlur = nullptr);
		void renderRest();
	public:
		Renderer(HWND hwnd, CRect rectView, Config* pConfig, Camera* pCamera, RenderableManager* pRenderableManager);
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
		bool getWireframe() const;
		void setWireframe(bool bWireframe);
		void toggleWireframe();
#define GETTER(name, type, t) type get##name() const { return m_##t##name; }
#define SETTER(name, type, t) void set##name(type t##name) { m_##t##name = t##name; }
#define TOGGLE(name) void toggle##name() { m_b##name = !m_b##name; }
#define GETSET(name, type, t) GETTER(name, type, t) SETTER(name, type, t)
#define GST_BOOL(name) GETSET(name, bool, b) TOGGLE(name)
		GST_BOOL(Tessellation)
		GST_BOOL(Bump)
		GST_BOOL(SSS)
		GST_BOOL(PostProcessAA)
		GST_BOOL(VSMBlur)

		void dump();

		// IRenderer implementation
		ID3D11Device* getDevice() const override;
		ID3D11DeviceContext* getDeviceContext() const override;
		void setWorldMatrix(const XMMATRIX& matWorld) override;
		void setMaterial(const Material& material) override;
		void useTexture(ID3D11SamplerState* pTextureSamplerState, ID3D11ShaderResourceView* pTexture) override;
		void usePlaceholderTexture() override;
		void useBumpMap(ID3D11SamplerState* pBumpMapSamplerState, ID3D11ShaderResourceView* pBumpMap) override;
		void usePlaceholderBumpMap() override;
		void useNormalMap(ID3D11SamplerState* pNormalMapSamplerState, ID3D11ShaderResourceView* pNormalMap) override;
		void usePlaceholderNormalMap() override;
		void setTessellationFactor(float edge, float inside, float min, float desiredSizeInPixels) override;
	};

} // namespace Skin
