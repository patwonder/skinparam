/**
 * Head renderer
 */

#pragma once

#include "MeshRenderable.h"

namespace Skin {
	class Head : public MeshRenderable {
	protected:
		XMMATRIX getWorldMatrix() const override;
	public:
		Head();
		~Head() override;

		void render(ID3D11DeviceContext* pDeviceContext, IRenderer* pRenderer, const Camera& pCamera) override;
		bool supportsSSS() const override { return true; }
		Utils::TString getName() const { return _T("Head"); }
	};
} // namespace Skin
