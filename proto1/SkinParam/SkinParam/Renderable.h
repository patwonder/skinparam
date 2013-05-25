/**
 * Interface for renderable objects
 */

#pragma once

#include "Camera.h"
#include "TString.h"
#include "D3DHelper.h"
#include "Material.h"
#include "FVector.h"

namespace Skin {
	class IRenderer /* interface */ {
	public:
		virtual ID3D11Device* getDevice() const = 0;
		virtual ID3D11DeviceContext* getDeviceContext() const = 0;
		virtual void setWorldMatrix(const XMMATRIX& matWorld) = 0;
		virtual void setMaterial(const Material& material) = 0;
		virtual void useTexture(ID3D11SamplerState* pTextureSamplerState, ID3D11ShaderResourceView* pTexture) = 0;
		virtual void usePlaceholderTexture() = 0;
		virtual void useBumpMap(ID3D11SamplerState* pBumpMapSamplerState, ID3D11ShaderResourceView* pBumpMap) = 0;
		virtual void usePlaceholderBumpMap() = 0;
		virtual void useNormalMap(ID3D11SamplerState* pNormalMapSamplerState, ID3D11ShaderResourceView* pNormalMap) = 0;
		virtual void usePlaceholderNormalMap() = 0;
		virtual void setTessellationFactor(float edge, float inside, float min, float desiredSizeInPixels) = 0;

		static const XMFLOAT4 COPY_DEFAULT_SCALE_FACTOR;
		static const XMFLOAT4 COPY_DEFAULT_VALUE;
		static const XMFLOAT4 COPY_DEFAULT_LERPS;

		virtual void dumpIrregularResourceToFile(ID3D11ShaderResourceView* pSRV, const Utils::TString& strFileName, bool overrideAutoNaming = false,
			XMFLOAT4 scaleFactor = COPY_DEFAULT_SCALE_FACTOR,
			XMFLOAT4 defaultValue = COPY_DEFAULT_VALUE,
			XMFLOAT4 lerps = COPY_DEFAULT_LERPS,
			DXGI_FORMAT preferredFormat = DXGI_FORMAT_UNKNOWN, DXGI_FORMAT preferredRTFormat = DXGI_FORMAT_UNKNOWN) = 0;
		virtual void dumpIrregularResourceToFile(ID3D11RenderTargetView* pRT, const Utils::TString& strFileName, bool overrideAutoNaming = false,
			XMFLOAT4 scaleFactor = COPY_DEFAULT_SCALE_FACTOR,
			XMFLOAT4 defaultValue = COPY_DEFAULT_VALUE,
			XMFLOAT4 lerps = COPY_DEFAULT_LERPS,
			DXGI_FORMAT preferredFormat = DXGI_FORMAT_UNKNOWN, DXGI_FORMAT preferredRTFormat = DXGI_FORMAT_UNKNOWN) = 0;
		virtual void dumpIrregularResourceToFile(ID3D11Resource* pResource, const Utils::TString& strFileName, bool overrideAutoNaming = false,
			XMFLOAT4 scaleFactor = COPY_DEFAULT_SCALE_FACTOR,
			XMFLOAT4 defaultValue = COPY_DEFAULT_VALUE,
			XMFLOAT4 lerps = COPY_DEFAULT_LERPS,
			DXGI_FORMAT preferredFormat = DXGI_FORMAT_UNKNOWN, DXGI_FORMAT preferredRTFormat = DXGI_FORMAT_UNKNOWN) = 0;

		virtual void dumpResourceToFile(ID3D11ShaderResourceView* pSRV, const Utils::TString& strFileName, bool overrideAutoNaming = false,
			DXGI_FORMAT preferredFormat = DXGI_FORMAT_UNKNOWN) = 0;
		virtual void dumpResourceToFile(ID3D11RenderTargetView* pRT, const Utils::TString& strFileName, bool overrideAutoNaming = false,
			DXGI_FORMAT preferredFormat = DXGI_FORMAT_UNKNOWN) = 0;
		virtual void dumpResourceToFile(ID3D11Resource* pResource, const Utils::TString& strFileName, bool overrideAutoNaming = false,
			DXGI_FORMAT preferredFormat = DXGI_FORMAT_UNKNOWN) = 0;
	};

	class Renderable /* interface */ {
	public:
		Renderable() {}

		/** must-implements */
		virtual void init(ID3D11Device* pDevice, IRenderer* pRenderer) = 0;
		virtual void render(ID3D11DeviceContext* pDeviceContext, IRenderer* pRenderer, const Camera& pCamera) = 0;
		virtual void cleanup(IRenderer* pRenderer) = 0;
		virtual ~Renderable() {}

		/** optional to implement */
		virtual bool supportsSSS() const { return false; }
		virtual bool inScene() const { return true; }
		virtual void getBoundingSphere(Utils::FVector& oVecCenter, float& oRadius) const {
			oVecCenter = Utils::FVector::ZERO;
			oRadius = 0.0f;
		}
	};

} // namespace Skin
