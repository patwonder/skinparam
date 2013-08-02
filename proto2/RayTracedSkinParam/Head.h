/*
 * The head object
 */

#pragma once

#include "MeshDrawable.h"

namespace RLSkin {
	class Head : public MeshDrawable {
	protected:
		XMMATRIX getWorldMatrix() const override {
			XMMATRIX mtWorld = XMMatrixScaling(10.0f, 10.0f, 10.0f);
			mtWorld *= XMMatrixRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), (float)(Utils::Math::PI / 2));
			mtWorld *= XMMatrixTranslation(0.0f, 0.0f, 0.2f);
			return mtWorld;
		}
		Utils::TString getName() const override {
			return _T("Head");
		}
	public:
		Head() : MeshDrawable(_T("model\\head.OBJ")) { }
	};
} // namespace RLSkin
