/**
 * Main renderer class
 */

#pragma once

#include <vector>

#include "Renderable.h"
#include "RenderableManager.h"
#include "Color.h"
#include "Frustum.h"

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

		CComPtr<ID3D11Device> m_pDevice;
		CComPtr<ID3D11DeviceContext> m_pDeviceContext;
		CComPtr<IDXGISwapChain> m_pSwapChain;
		CComPtr<ID3D11RenderTargetView> m_pRenderTargetView;
		CComPtr<ID3D11Texture2D> m_pDepthStencil;
		CComPtr<ID3D11DepthStencilView> m_pDepthStencilView;
		CComPtr<ID3D11SamplerState> m_pPlaceholderSamplerState;
		CComPtr<ID3D11ShaderResourceView> m_pPlaceholderTexture;
		CComPtr<ID3D11SamplerState> m_pBumpSamplerState;
		CComPtr<ID3D11ShaderResourceView> m_pBumpTexture;
		CComPtr<ID3D11ShaderResourceView> m_pNormalTexture;
		CComPtr<ID3D11SamplerState> m_pLinearSamplerState;
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
		static const UINT SSS_ATTENUATION_TEXTURE_SIZE = 256;
		CComPtr<ID3D11RenderTargetView> m_apRTSSS[NUM_SSS_VIEWS];
		CComPtr<ID3D11ShaderResourceView> m_apSRVSSS[NUM_SSS_VIEWS];
		CComPtr<ID3D11RenderTargetView> m_pRTAttenuationTexture;
		CComPtr<ID3D11ShaderResourceView> m_pSRVAttenuationTexture;
		D3D11_VIEWPORT m_vpAttenuationTexture;
		ShaderGroup* m_psgSSSIrradiance;
		ShaderGroup* m_psgSSSIrradianceNoTessellation;
		ShaderGroup* m_psgSSSIrradianceNoGaussian;
		ShaderGroup* m_psgSSSIrradianceNoGaussianAA;
		ShaderGroup* m_psgSSSIrradianceNoTessellationNoGaussian;
		ShaderGroup* m_psgSSSIrradianceNoTessellationNoGaussianAA;
		ShaderGroup* m_psgSSSGausianVertical;
		ShaderGroup* m_psgSSSGausianHorizontal;
		ShaderGroup* m_psgSSSGausianVertical7;
		ShaderGroup* m_psgSSSGausianHorizontal7;
		ShaderGroup* m_psgSSSGausianVertical5;
		ShaderGroup* m_psgSSSGausianHorizontal5;
		ShaderGroup* m_psgSSSGausianVertical3;
		ShaderGroup* m_psgSSSGausianHorizontal3;
		ShaderGroup* m_psgSSSCombine;
		ShaderGroup* m_psgSSSCombineAA;
		ShaderGroup* m_psgSSSAttenuationTexture;
		static const float SSS_GAUSSIAN_KERNEL_SIGMA[NUM_SSS_GAUSSIANS];
		static const float SSS_GAUSSIAN_WEIGHTS[NUM_SSS_GAUSSIANS][3];

		struct GaussianConstantBuffer {
			float g_blurWidth; // blur width
			float g_invAspectRatio; // inverse of aspect ratio (height / width)
			float g_screenWidth;
			float g_screenHeight;
		};
		struct CombineConstantBuffer {
			struct {
				XMFLOAT3 value;
				float pad;
			} g_weights[NUM_SSS_GAUSSIANS];
		};
		struct SSSConstantBuffer {
			float g_sss_intensity;
			float g_sss_strength;
			float pad[2];
		};

		CComPtr<ID3D11Buffer> m_pGaussianConstantBuffer;
		GaussianConstantBuffer m_cbGaussian;
		CComPtr<ID3D11Buffer> m_pCombineConstantBuffer;
		CombineConstantBuffer m_cbCombine;
		CComPtr<ID3D11Buffer> m_pSSSConstantBuffer;
		SSSConstantBuffer m_cbSSS;

		// Post-process Anti-aliasing
		ShaderGroup* m_psgPostProcessAA;

		struct PostProcessConstantBuffer {
			XMFLOAT2 rcpFrame;
			float pad[2];
			XMFLOAT4 rcpFrameOpt;
		};
		CComPtr<ID3D11Buffer> m_pPostProcessConstantBuffer;
		PostProcessConstantBuffer m_cbPostProcess;

		std::vector<ShaderGroup**> m_vppShaderGroups;

		static const UINT SLOT_TEXTURE = 0;
		static const UINT SLOT_BUMPMAP = 1;
		static const UINT SLOT_NORMALMAP = 2;
		static const UINT SLOT_ATTENUATION = 3;
		static const UINT SLOT_SHADOWMAP = 4;
		static const UINT SLOT_SHADOWMAPDEPTH = 5;

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
			XMFLOAT4 g_vFrustumPlaneEquation[4];
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
		CComPtr<ID3D11Buffer> m_pTransformConstantBuffer;
		TransformConstantBuffer m_cbTransform;
		CComPtr<ID3D11Buffer> m_pLightingConstantBuffer;
		LightingConstantBuffer m_cbLighting;
		CComPtr<ID3D11Buffer> m_pMaterialConstantBuffer;
		MaterialConstantBuffer m_cbMaterial;
		CComPtr<ID3D11Buffer> m_pTessellationConstantBuffer;
		TessellationConstantBuffer m_cbTessellation;

		static const float CLIPPING_LIGHT_NEAR;
		static const float CLIPPING_LIGHT_FAR;
		static const float CLIPPING_SCENE_NEAR;
		static const float CLIPPING_SCENE_FAR;
		static const float FOV_SCENE;
		static const float FOV_LIGHT;
		static const float MM_PER_LENGTH;
		XMFLOAT4X4 m_matProjection; // use XMFLOAT4X4 instead of XMMATRIX to resolve alignment issues
		XMFLOAT4X4 m_matLightProjection; // light projection matrix
		Frustum m_frustum;

		// Shadow mapping
		static const UINT SM_SIZE = 2048;
		static const UINT NUM_SHADOW_VIEWS = NUM_LIGHTS;
		D3D11_VIEWPORT m_vpShadowMap;
		CComPtr<ID3D11RenderTargetView> m_apRTShadowMaps[NUM_SHADOW_VIEWS];
		CComPtr<ID3D11ShaderResourceView> m_apSRVShadowMaps[NUM_SHADOW_VIEWS];
		CComPtr<ID3D11DepthStencilView> m_pShadowMapDepthStencilView;
		CComPtr<ID3D11SamplerState> m_pShadowMapSamplerState;
		CComPtr<ID3D11SamplerState> m_pShadowMapDepthSamplerState;

		// Bloom filter
		ShaderGroup* m_psgBloomDetect;
		ShaderGroup* m_psgBloomVertical;
		ShaderGroup* m_psgBloomHorizontal;
		ShaderGroup* m_psgBloomCombine;
		ShaderGroup* m_psgBloomCombineAA;
		struct BloomConstantBuffer {
			XMFLOAT2 rcpScreenSize;
			UINT sampleLevel;
			float pad;
		};
		CComPtr<ID3D11Buffer> m_pBloomConstantBuffer;
		BloomConstantBuffer m_cbBloom;
		CComPtr<ID3D11BlendState> m_pBSAdditiveBlending;
		CComPtr<ID3D11BlendState> m_pBSNoBlending;
		static const UINT BLOOM_PASSES = 6;
		CComPtr<ID3D11ShaderResourceView> m_pSRVBloomSource;
		CComPtr<ID3D11RenderTargetView> m_pRTBloomSource;
		CComPtr<ID3D11ShaderResourceView> m_pSRVBloomDetect;
		CComPtr<ID3D11RenderTargetView> m_pRTBloomDetect;
		CComPtr<ID3D11ShaderResourceView> m_pSRVBloomTemporary;
		CComPtr<ID3D11RenderTargetView> m_pRTBloomTemporary;
		CComPtr<ID3D11ShaderResourceView> m_apSRVBloom[BLOOM_PASSES];
		CComPtr<ID3D11ShaderResourceView> m_apSRVBloomTemp[BLOOM_PASSES];
		CComPtr<ID3D11RenderTargetView> m_apRTBloom[BLOOM_PASSES];
		CComPtr<ID3D11RenderTargetView> m_apRTBloomTemp[BLOOM_PASSES];
		D3D11_VIEWPORT m_avpBloom[BLOOM_PASSES];

		// Copy
		ShaderGroup* m_psgCopy;
		ShaderGroup* m_psgCopyLinear;

		struct CopyConstantBuffer {
			XMFLOAT4 scaleFactor;
			XMFLOAT4 defaultValue;
			XMFLOAT4 lerps;
		};
		CComPtr<ID3D11Buffer> m_pCopyConstantBuffer;
		CopyConstantBuffer m_cbCopy;
		static const XMFLOAT4 COPY_DEFAULT_SCALE_FACTOR;
		static const XMFLOAT4 COPY_DEFAULT_VALUE;
		static const XMFLOAT4 COPY_DEFAULT_LERPS;

		// View-In-View
		ShaderGroup* m_psgCanvas;
		ShaderGroup* m_psgCanvasAA;
		D3D11_VIEWPORT m_vpViewInView;
		CComPtr<ID3D11Buffer> m_pVIVVertexBuffer;
		struct CanvasVertex {
			XMFLOAT3 position;
			XMFLOAT4 color;
		};

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
		bool m_bAdaptiveGaussian;
		bool m_bBloom;
		bool m_bDump;
		UINT m_nDumpCount;

		static const TCHAR* DEFAULT_DUMP_FOLDER;

		// render stage control
		enum RenderStage {
			RS_NotRendering,
			RS_ShadowMap,
			RS_Irradiance,
			RS_Gaussian,
			RS_Combine,
			RS_Bloom,
			RS_PostProcessAA,
			RS_UI
		} m_rsCurrent;

		// pre-rendering
		bool m_bPRAttenuationTexture;

		HRESULT initDX();
		void initMisc();

		void loadShaders();
		void unloadShaders();

		void initViewInView();
		void initTransform();
		XMMATRIX getViewProjMatrix(const Camera& camera, const XMMATRIX& matProjection, XMMATRIX& matView);
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
		void doPreRenderings();
		void bindAttenuationTexture();
		void unbindAttenuationTexture();
		void initBloom();
		void doBloom();

		void initSSS();
		void getImmediateIrradianceRT(ID3D11RenderTargetView** ppRT, ShaderGroup** ppsg, float* palpha);
		void renderIrradianceMap(bool* opbNeedBlur = nullptr);
		void renderViewInView(const Camera* pCamera);
		void setGaussianKernelSize(float size);
		void unbindInputBuffers();
		void bindGaussianConstantBuffer();
		void bindCombineConstantBuffer();
		float getMinDepthForScene() const;
		float estimateGaussianKernelSize(float standardDeviation, float minDepth);
		bool selectShaderGroupsForKernelSize(float kernelSizeInPixels, ShaderGroup** ppsgVertical, ShaderGroup** ppsgHorizontal);
		void doGaussianBlurs(ID3D11ShaderResourceView* oapSRVGaussians[]);
		void doCombineShading(bool bNeedBlur, ID3D11ShaderResourceView* apSRVGaussians[]);

		void initPostProcessAA();
		void bindPostProcessConstantBuffer();
		void doPostProcessAA();

		void initCopy();

		void copyRender(ID3D11ShaderResourceView* pSRV, ID3D11RenderTargetView* pRT, bool bLinear = false,
			XMFLOAT4 scaleFactor = COPY_DEFAULT_SCALE_FACTOR,
			XMFLOAT4 defaultValue = COPY_DEFAULT_VALUE,
			XMFLOAT4 lerps = COPY_DEFAULT_LERPS);

		void dumpIrregularResourceToFile(ID3D11ShaderResourceView* pSRV, const Utils::TString& strFileName, bool overrideAutoNaming = false,
			XMFLOAT4 scaleFactor = COPY_DEFAULT_SCALE_FACTOR,
			XMFLOAT4 defaultValue = COPY_DEFAULT_VALUE,
			XMFLOAT4 lerps = COPY_DEFAULT_LERPS,
			DXGI_FORMAT preferredFormat = DXGI_FORMAT_UNKNOWN, DXGI_FORMAT preferredRTFormat = DXGI_FORMAT_UNKNOWN);
		void dumpIrregularResourceToFile(ID3D11RenderTargetView* pRT, const Utils::TString& strFileName, bool overrideAutoNaming = false,
			XMFLOAT4 scaleFactor = COPY_DEFAULT_SCALE_FACTOR,
			XMFLOAT4 defaultValue = COPY_DEFAULT_VALUE,
			XMFLOAT4 lerps = COPY_DEFAULT_LERPS,
			DXGI_FORMAT preferredFormat = DXGI_FORMAT_UNKNOWN, DXGI_FORMAT preferredRTFormat = DXGI_FORMAT_UNKNOWN);
		void dumpIrregularResourceToFile(ID3D11Resource* pResource, const Utils::TString& strFileName, bool overrideAutoNaming = false,
			XMFLOAT4 scaleFactor = COPY_DEFAULT_SCALE_FACTOR,
			XMFLOAT4 defaultValue = COPY_DEFAULT_VALUE,
			XMFLOAT4 lerps = COPY_DEFAULT_LERPS,
			DXGI_FORMAT preferredFormat = DXGI_FORMAT_UNKNOWN, DXGI_FORMAT preferredRTFormat = DXGI_FORMAT_UNKNOWN);

		void dumpResourceToFile(ID3D11ShaderResourceView* pSRV, const Utils::TString& strFileName, bool overrideAutoNaming = false,
			DXGI_FORMAT preferredFormat = DXGI_FORMAT_UNKNOWN);
		void dumpResourceToFile(ID3D11RenderTargetView* pRT, const Utils::TString& strFileName, bool overrideAutoNaming = false,
			DXGI_FORMAT preferredFormat = DXGI_FORMAT_UNKNOWN);
		void dumpResourceToFile(ID3D11Resource* pResource, const Utils::TString& strFileName, bool overrideAutoNaming = false,
			DXGI_FORMAT preferredFormat = DXGI_FORMAT_UNKNOWN);

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
		Utils::Color getGlobalAmbient() const;
		void addLight(Light* light);
		void removeLight(Light* light);
		void removeAllLights();

		void render(const Camera* pSecondaryView = nullptr);

		// Statistics
		float getFPS() const { return m_fps; }

		// Toggleables
		bool getWireframe() const;
		void setWireframe(bool bWireframe);
#define GETTER(name, type, t) type get##name() const { return m_##t##name; }
#define SETTER(name, type, t) void set##name(type t##name) { m_##t##name = t##name; }
#define TOGGLE(name) void toggle##name() { set##name(!get##name()); }
#define GETSET(name, type, t) GETTER(name, type, t) SETTER(name, type, t)
#define GST_BOOL(name) GETSET(name, bool, b) TOGGLE(name)
		TOGGLE(Wireframe)
		GST_BOOL(Tessellation)
		GST_BOOL(Bump)
		GST_BOOL(SSS)
		GST_BOOL(VSMBlur)
		GETTER(PostProcessAA, bool, b) TOGGLE(PostProcessAA)
		void setPostProcessAA(bool bPostProcessAA);
		GST_BOOL(AdaptiveGaussian)
		GST_BOOL(Bloom)

		float getSSSStrength() const { return m_cbSSS.g_sss_strength; }
		void setSSSStrength(float strength) { m_cbSSS.g_sss_strength = strength; }
		void dump();

		D3D_DRIVER_TYPE getDriverType() const { return m_driverType; }

		Camera getLightCamera(const Light& light);

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
