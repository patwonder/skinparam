/**
 * Renderable that capable of rendering meshes
 */

#pragma once

#include "Renderable.h"
#include <string>
#include <map>
#include <unordered_map>

namespace Utils {
	class ObjModel;
	struct ObjTexCoord;
	struct ObjPart;
}

namespace Skin {

	class MeshRenderable : public Renderable {
	private:
		Utils::FVector m_vCenter;
		float m_fBoundingSphereRadius;
		std::unordered_map<UINT, float> m_mapContourBump;

		static float getBumpMultiplierScale(const XMMATRIX& matWorld);

		void removeDuplicateVertices();
		void computeNormals();
		void computeBoundingSphere();
		void computeTangentSpace();
		void detectContourVertices();
		void detectContourVerticesForPart(const Utils::ObjPart& part);
		void computeNormalMaps(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);

		typedef UINT16 BTT;
		static const DXGI_FORMAT BumpTexFormat = DXGI_FORMAT_R16_UNORM;
		static const WICPixelFormatGUID BumpTexWICFormat;
		static const BTT BTTUpper = 65535;
		typedef XMFLOAT2 NTT;
		static const DXGI_FORMAT NormTexFormat = DXGI_FORMAT_R32G32_FLOAT;
		static const float NormalDistance;

		struct BumpMapSampleData {
			BTT* pData;
			UINT width;
			UINT height;

			BumpMapSampleData();
			~BumpMapSampleData();
		};

		float sampleBumpMap(BTT** ppData, UINT* pWidth, UINT* pHeight, Utils::ObjTexCoord texCoord, const std::string& materialName);
		void computeNormalMap(const BTT* pBumpTextureData, UINT width, UINT height, NTT* pNormalMapData);
	protected:
		struct Vertex {
			XMFLOAT3 position;
			XMFLOAT4 color;
			XMFLOAT3 normal;
			XMFLOAT3 tangent;
			XMFLOAT3 binormal;
			XMFLOAT2 texCoord;
			XMFLOAT2 bumpOverride;
		};

		Utils::ObjModel* m_pModel;
		float m_roughness;

		CComPtr<ID3D11Buffer> m_pVertexBuffer;
		CComPtr<ID3D11Buffer> m_pIndexBuffer;
		CComPtr<ID3D11SamplerState> m_pSamplerState;
		std::map<std::string, CComPtr<ID3D11ShaderResourceView> > m_vpTextures;
		std::map<std::string, CComPtr<ID3D11ShaderResourceView> > m_vpBumpMaps;
		std::map<std::string, CComPtr<ID3D11ShaderResourceView> > m_vpNormalMaps;

		MeshRenderable(const Utils::TString& strObjFilePath);
		~MeshRenderable() override;

		virtual XMMATRIX getWorldMatrix() const = 0;
		virtual Utils::TString getName() const = 0;
	public:
		void init(ID3D11Device* pDevice, IRenderer* pRenderer) override;
		void render(ID3D11DeviceContext* pDeviceContext, IRenderer* pRenderer, const Camera& pCamera) override;
		void cleanup(IRenderer* pRenderer) override;

		void getBoundingSphere(Utils::FVector& oVecCenter, float& oRadius) const override;
		void getBoundingBox(Utils::FVector& oVecLower, Utils::FVector& oVecUpper) const;

		void setRoughness(float roughness) { m_roughness = roughness; }
		float getRoughness() const { return m_roughness; }
	};
} // namespace Skin
