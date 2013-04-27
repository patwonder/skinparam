/**
 * Interface for renderable objects
 */

#pragma once

#include "Camera.h"
#include "TString.h"
#include "D3DHelper.h"
#include "Material.h"

namespace Skin {
	class IRenderer /* interface */ {
	public:
		virtual void setWorldMatrix(const XMMATRIX& matWorld) = 0;
		virtual void setMaterial(const Material& material) = 0;
		virtual void usePlaceholderTexture() = 0;
		virtual void setTessellationFactor(float edge, float inside, float min, float desiredSize) = 0;
	};

	class Renderable /* interface */ {
	public:
		Renderable() {}

		/** must-implements */
		virtual void init(ID3D11Device* pDevice, IRenderer* pRenderer) = 0;
		virtual void render(ID3D11DeviceContext* pDeviceContext, IRenderer* pRenderer, const Camera& pCamera) = 0;
		virtual void cleanup(IRenderer* pRenderer) = 0;
		virtual ~Renderable() {}
	};

} // namespace Skin
