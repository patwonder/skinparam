/**
 * Test renderable object
 */
#pragma once

#include "Renderable.h"

namespace Skin {
	class Triangle : public Renderable {
	private:
		ID3D11VertexShader* m_pVertexShader;
		ID3D11PixelShader* m_pPixelShader;
		ID3D11InputLayout* m_pInputLayout;
		ID3D11Buffer* m_pVertexBuffer;
	public:
		Triangle();
		void init(ID3D11Device* pDevice) override;
		void render(ID3D11DeviceContext* pDeviceContext, const Camera& pCamera) override;
		void cleanup() override;
		~Triangle() override;
	};
} // namespace Skin
