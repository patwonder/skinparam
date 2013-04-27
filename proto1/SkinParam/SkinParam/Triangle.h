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
		void init(ID3D11Device* pDevice, IRenderer* pRenderer) override;
		void render(ID3D11DeviceContext* pDeviceContext, IRenderer* pRenderer, const Camera& pCamera) override;
		void cleanup(IRenderer* pRenderer) override;
		~Triangle() override;
	};
} // namespace Skin
