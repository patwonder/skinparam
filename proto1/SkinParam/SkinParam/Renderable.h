
/*
    Copyright(c) 2013-2014 Yifan Wu.

    This file is part of SkinParam.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

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
