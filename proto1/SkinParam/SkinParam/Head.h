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
	};
} // namespace Skin
