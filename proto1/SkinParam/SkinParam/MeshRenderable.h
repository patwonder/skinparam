/**
 * Renderable that capable of rendering meshes
 */

#pragma once

#include "Renderable.h"
#include <string>
#include <map>

namespace Utils {
	class ObjModel;
}

namespace Skin {

	class MeshRenderable : public Renderable {
	private:
		void removeDuplicateVertices();
		void computeNormals();
	protected:
		struct Vertex {
			XMFLOAT3 position;
			XMFLOAT4 color;
			XMFLOAT3 normal;
			XMFLOAT2 texCoord;
		};

		Utils::ObjModel* m_pModel;

		ID3D11Buffer* m_pVertexBuffer;
		ID3D11Buffer* m_pIndexBuffer;
		ID3D11SamplerState* m_pSamplerState;
		std::map<std::string, ID3D11ShaderResourceView*> m_vpTextures;
		std::map<std::string, ID3D11ShaderResourceView*> m_vpBumpMaps;

		MeshRenderable(const Utils::TString& strObjFilePath);
		~MeshRenderable() override;

		virtual XMMATRIX getWorldMatrix() const = 0;
	public:
		void init(ID3D11Device* pDevice, IRenderer* pRenderer) override;
		void render(ID3D11DeviceContext* pDeviceContext, IRenderer* pRenderer, const Camera& pCamera) override;
		void cleanup(IRenderer* pRenderer) override;
	};
} // namespace Skin
