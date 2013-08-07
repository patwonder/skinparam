/*
 * Drawables that renders triangle meshes
 */

#pragma once

#include "Drawable.h"
#include "FVector.h"
#include "ObjLoader.h"
#include "Texture.h"
#include "Primitive.h"
#include "Buffer.h"
#include "RLPtr.h"

namespace RLSkin {
	class MeshDrawable : public Drawable {
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
		void computeNormalMaps();

		typedef UINT16 BTT;
		static const DXGI_FORMAT BumpTexFormat = DXGI_FORMAT_R16_UNORM;
		static const WICPixelFormatGUID BumpTexWICFormat;
		static const BTT BTTUpper = 65535;
		typedef XMFLOAT4 NTT;
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
		Utils::ObjModel* m_pModel;

		virtual XMMATRIX getWorldMatrix() const = 0;
		virtual Utils::TString getName() const = 0;

		struct {
			RLPtr<Buffer> pPositionBuffer;
			RLPtr<Buffer> pNormalBuffer;
			RLPtr<Buffer> pTangentBuffer;
			RLPtr<Buffer> pBinormalBuffer;
			RLPtr<Buffer> pTexCoordBuffer;
		} m_buffers;
		std::vector<RLPtr<Primitive> > m_vpPrimitives;
		std::unordered_map<std::string, RLPtr<Texture> > m_vpTextures;
		std::unordered_map<std::string, RLPtr<Texture> > m_vpBumpMaps;
		std::unordered_map<std::string, RLPtr<Texture> > m_vpNormalMaps;
	public:
		MeshDrawable(const Utils::TString& strObjFilePath);
		~MeshDrawable() override;

		void init() override;
		void draw(Program* pProgram, Buffer* pLightBuffer, Primitive* pEnvironmentPrimitive) override;
		void cleanup() override;

		void getBoundingSphere(Utils::FVector& oVecCenter, float& oRadius) const override;
	};
} // namespace RLSkin
