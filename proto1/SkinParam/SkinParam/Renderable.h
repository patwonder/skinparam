/**
 * Interface for renderable objects
 */

#pragma once

#include "Camera.h"
#include "TString.h"
#include "D3DHelper.h"

namespace Skin {
	class Renderable /* interface */ {
	public:
		/** must-implements */
		virtual void init(ID3D11Device* pDevice) = 0;
		virtual void render(ID3D11DeviceContext* pDeviceContext, const Camera& pCamera) = 0;
		virtual void cleanup() = 0;
		virtual ~Renderable() {}

		/** auxillary features */
		virtual bool useTransform() const { return false; }
		virtual XMMATRIX getWorldMatrix() const;
	};

} // namespace Skin
