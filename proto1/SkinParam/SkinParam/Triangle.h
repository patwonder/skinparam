/**
 * Test renderable object
 */
#pragma once

#include "Renderable.h"

namespace Skin {
	class Triangle : public Renderable {
	private:
		ID3D11Buffer* m_pVertexBuffer;
		ID3D11Buffer* m_pIndexBuffer;
	public:
		Triangle();
		void init(ID3D11Device* pDevice) override;
		void render(ID3D11DeviceContext* pDeviceContext, const Camera& pCamera) override;
		void cleanup() override;
		~Triangle() override;

		bool useTransform() const override { return true; }
		XMMATRIX getWorldMatrix() const override;
		Material getMaterial() const override;
	};
} // namespace Skin
