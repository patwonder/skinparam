/**
 * Renderable that capable of rendering meshes
 */

#include "stdafx.h"

#include "MeshRenderable.h"
#include "ObjLoader.h"
#include "D3DHelper.h"
#include <unordered_map>
#include <unordered_set>

using namespace Skin;
using namespace Utils;
using namespace D3DHelper;

const float MeshRenderable::NormalDistance = 0.04f;

MeshRenderable::MeshRenderable(const TString& strObjFilePath)
	: m_pVertexBuffer(nullptr), m_pIndexBuffer(nullptr), m_pSamplerState(nullptr)
{
	m_pModel = nullptr;

	std::string path = ANSIStringFromTString(strObjFilePath);
	ObjLoader loader(path);

	m_pModel = loader.ReturnObj();
	computeNormals();
	computeTangentSpace();

	std::map<UINT, UINT> mapVertexToTexCoord;
	for (const ObjPart& part : m_pModel->Parts) {
		for (int idxTri = part.TriIdxMin; idxTri < part.TriIdxMax; idxTri++) {
			ObjTriangle& tri = m_pModel->Triangles[idxTri];
			for (int j = 0; j < 3; j++) {
				auto iter = mapVertexToTexCoord.find(tri.Vertex[j]);
				if (iter != mapVertexToTexCoord.end() && iter->second != tri.TexCoord[j]) {
					TRACE(_T("BOOM!"));
				} else {
					mapVertexToTexCoord[tri.Vertex[j]] = tri.TexCoord[j];
				}
			}
		}
	}
}

MeshRenderable::~MeshRenderable() {
	delete m_pModel;
}

void MeshRenderable::init(ID3D11Device* pDevice, IRenderer* pRenderer) {
/**
 * The original rendering loop, for reference
 	for (std::vector<ObjPart>::const_iterator iter = m_objModel->Parts.begin();
		iter != m_objModel->Parts.end(); ++iter) {
			// setup material
			std::string mtlname = iter->MaterialName;
			std::map<std::string, ObjMaterial>::const_iterator iterMtl;
			if (m_objModel->Materials.end() != (iterMtl = m_objModel->Materials.find(mtlname))) {
				const ObjMaterial& material = iterMtl->second;
				if (params.f_material.Ke) {
				// Set materials
				}
			}
			// draw the object part
			glBegin(GL_TRIANGLES); {
				for (int idxTri = iter->TriIdxMin; idxTri < iter->TriIdxMax; idxTri++) {
					const ObjTriangle& triangle = m_objModel->Triangles[idxTri];
					for (int j = 0; j < 3; j++) {
						if (triangle.TexCoord[j])
							glTexCoord2fv(&m_objModel->TexCoords[triangle.TexCoord[j]].U);
						if (triangle.Normal[j])
							glNormal3fv(&m_objModel->Normals[triangle.Normal[j]].x);
						if (triangle.Vertex[j])
							glVertex3fv(&m_objModel->Vertices[triangle.Vertex[j]].x);
					}
				}
			} glEnd();
	}
 */
	// for each triangle we insert a vertex into the vertex buffer
	// compute number of vertices
	UINT numVertices = 0;
	for (const ObjPart& part : m_pModel->Parts) {
		numVertices += 3 * (part.TriIdxMax - part.TriIdxMin);
	}

	// create vertex array
	Vertex* vertices = new Vertex[numVertices];

	// the (fake) rendering loop
	UINT i = 0;
	for (const ObjPart& part : m_pModel->Parts) {
		for (int idxTri = part.TriIdxMin; idxTri < part.TriIdxMax; idxTri++) {
			const ObjTriangle& tri = m_pModel->Triangles[idxTri];
			for (int j = 0; j < 3; j++) {
				Vertex& v = vertices[i];
				v.color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
				v.position = XMFLOAT3(m_pModel->Vertices[tri.Vertex[j]].toArray());
				v.normal = XMFLOAT3(m_pModel->Normals[tri.Normal[j]].toArray());
				v.binormal = XMFLOAT3(m_pModel->Binormals[tri.Vertex[j]].toArray());
				v.tangent = XMFLOAT3(m_pModel->Tangents[tri.Vertex[j]].toArray());
				v.texCoord = XMFLOAT2(&m_pModel->TexCoords[tri.TexCoord[j]].U);
				i++;
			}
		}
	}

	checkFailure(createVertexBuffer(pDevice, vertices, numVertices, &m_pVertexBuffer),
		_T("Failed to create vertex buffer for mesh"));

	delete[] vertices;

	// Create sampler state
	checkFailure(createSamplerState(pDevice, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, &m_pSamplerState),
		_T("Failed to create sampler state"));

	// Create textures for each material
	for (const auto& objMtPair : m_pModel->Materials) {
		const ObjMaterial& objMt = objMtPair.second;
		if (objMt.TextureFileName.length()) {
			ID3D11ShaderResourceView* pTexture = nullptr;
			checkFailure(loadTexture(pDevice, pRenderer->getDeviceContext(), _T("model\\") + TStringFromANSIString(objMt.TextureFileName), &pTexture),
				_T("Unable to load texture ") + TStringFromANSIString(objMt.TextureFileName));
			m_vpTextures[objMtPair.first] = pTexture;
		}
		if (objMt.BumpMapFileName.length()) {
			ID3D11ShaderResourceView* pBumpMap = nullptr;
			checkFailure(loadTexture(pDevice, pRenderer->getDeviceContext(), _T("model\\") + TStringFromANSIString(objMt.BumpMapFileName), &pBumpMap),
				_T("Unable to load bump map ") + TStringFromANSIString(objMt.BumpMapFileName));
			m_vpBumpMaps[objMtPair.first] = pBumpMap;
		}
	}

	computeNormalMaps(pDevice, pRenderer->getDeviceContext());
}

namespace Skin {
	static std::hash<float> float_hasher;
	class ObjVertexHasher : public std::unary_function<const ObjVertex&, size_t> {
	public:
		size_t operator()(const ObjVertex& v) const {
			return float_hasher(v.x) ^ float_hasher(v.y) ^ float_hasher(v.z);
		}
	};
	class ObjVertexEqualTo : public std::binary_function<const ObjVertex&, const ObjVertex&, bool> {
	public:
		bool operator()(const ObjVertex& v1, const ObjVertex& v2) const {
			return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
		}
	};
};

void MeshRenderable::removeDuplicateVertices() {
	std::vector<UINT> vOldToNewIdx;
	std::unordered_map<ObjVertex, UINT, ObjVertexHasher, ObjVertexEqualTo> mapVerticesToIdx;
	const std::vector<ObjVertex>& vOldVertices = m_pModel->Vertices;
	std::vector<ObjVertex> vNewVertices;

	// copy over the first placeholder element
	vNewVertices.push_back(vOldVertices[0]);
	vOldToNewIdx.push_back(0);

	// insert vertices into the set
	for (UINT i = 1; i < vOldVertices.size(); i++) {
		const ObjVertex& vertex = vOldVertices[i];
		auto iter = mapVerticesToIdx.find(vertex);
		if (iter != mapVerticesToIdx.end()) {
			// already exists, map index i to recorded new index
			vOldToNewIdx.push_back(iter->second);
		} else {
			// does not exist yet, insert as a new vertex and record new index
			UINT newIdx = vNewVertices.size();
			mapVerticesToIdx[vertex] = newIdx;
			vOldToNewIdx.push_back(newIdx);
			vNewVertices.push_back(vertex);
		}
	}

	// index substitution
	for (const ObjPart& part : m_pModel->Parts) {
		for (int idxTri = part.TriIdxMin; idxTri < part.TriIdxMax; idxTri++) {
			ObjTriangle& tri = m_pModel->Triangles[idxTri];
			for (int j = 0; j < 3; j++) {
				tri.Vertex[j] = vOldToNewIdx[tri.Vertex[j]];
			}
		}
	}

	TRACE(_T("Removed %d duplicate vertices out of %d total vertices.\n"), vOldToNewIdx.size() - vNewVertices.size(), vOldToNewIdx.size() - 1);

	// vertices substitution
	m_pModel->Vertices = std::move(vNewVertices);
}

void MeshRenderable::computeNormals() {
	// quit if there's already some normals in the model
	// note normals (and vertices, texcoords) begin at index 1
	if (m_pModel->Normals.size() > 1) return;
	
	// avoid computing different normals for the same vertex
	//removeDuplicateVertices();

	UINT numVertices = m_pModel->Vertices.size() - 1;

	auto& normals = m_pModel->Normals;
	normals.resize(m_pModel->Vertices.size());
	// normals are automatically zeroed when FVectors are constructed

	for (const ObjPart& part : m_pModel->Parts) {
		for (int idxTri = part.TriIdxMin; idxTri < part.TriIdxMax; idxTri++) {
			const ObjTriangle& tri = m_pModel->Triangles[idxTri];
			const FVector& v1 = m_pModel->Vertices[tri.Vertex[0]];
			const FVector& v2 = m_pModel->Vertices[tri.Vertex[1]];
			const FVector& v3 = m_pModel->Vertices[tri.Vertex[2]];

			FVector normal = (v2 - v1).cross(v3 - v1);
			normals[tri.Vertex[0]] += normal;
			normals[tri.Vertex[1]] += normal;
			normals[tri.Vertex[2]] += normal;
		}
	}

	// calculate average normal
	for (UINT i = 1; i <= numVertices; i++) {
		if (normals[i])
			normals[i] = normals[i].normalize();
	}

	// apply normals to faces
	for (const ObjPart& part : m_pModel->Parts) {
		for (int idxTri = part.TriIdxMin; idxTri < part.TriIdxMax; idxTri++) {
			ObjTriangle& tri = m_pModel->Triangles[idxTri];
			for (int j = 0; j < 3; j++) {
				tri.Normal[j] = tri.Vertex[j];
			}
		}
	}
}

void MeshRenderable::computeTangentSpace() {
	// quit if there's already some tangents/binormals in the model
	// note tangents/binormals (and vertices, texcoords) begin at index 1
	if (m_pModel->Tangents.size() > 1 || m_pModel->Binormals.size() > 1) return;

	UINT numVertices = m_pModel->Vertices.size() - 1;
	
	// initialization
	std::vector<ObjNormal> normals;
	auto& tangents = m_pModel->Tangents;
	auto& binormals = m_pModel->Binormals;
	normals.resize(m_pModel->Vertices.size());
	tangents.resize(m_pModel->Vertices.size());
	binormals.resize(m_pModel->Vertices.size());
	// already zeroed when default constructors are called

	// See http://www.terathon.com/code/binormal.html
	for (const ObjPart& part : m_pModel->Parts) {
		for (int idxTri = part.TriIdxMin; idxTri < part.TriIdxMax; idxTri++) {
			const ObjTriangle& tri = m_pModel->Triangles[idxTri];
			const ObjTexCoord& t0 = m_pModel->TexCoords[tri.TexCoord[0]];
			const ObjTexCoord& t1 = m_pModel->TexCoords[tri.TexCoord[1]];
			const ObjTexCoord& t2 = m_pModel->TexCoords[tri.TexCoord[2]];
			const ObjVertex& v0 = m_pModel->Vertices[tri.Vertex[0]];
			const ObjVertex& v1 = m_pModel->Vertices[tri.Vertex[1]];
			const ObjVertex& v2 = m_pModel->Vertices[tri.Vertex[2]];

			// deltas
			ObjTexCoord dt1 = { t1.U - t0.U, t1.V - t0.V };
			ObjTexCoord dt2 = { t2.U - t0.U, t2.V - t0.V };
			FVector dv1 = v1 - v0;
			FVector dv2 = v2 - v0;

			// the formula
			//
			// | Tx Ty Tz |       1     |  t2 -t1 || Q1x Q1y Q1z |
			// |          | = _________ |         ||             |
			// | Bx By Bz |   s1t2-s2t1 | -s2  s1 || Q2x Q2y Q2z |
			//

			// triangle area * 2 in texture space
			float invda = -1.0f / (dt1.U * dt2.V - dt1.V * dt2.U);
			ObjTangent tan = ObjTangent(dt2.V * dv1.x - dt1.V * dv2.x, 
										 dt2.V * dv1.y - dt1.V * dv2.y,
										 dt2.V * dv1.z - dt1.V * dv2.z);
			ObjBinormal bn = ObjBinormal(-dt2.U * dv1.x + dt1.U * dv2.x,
										-dt2.U * dv1.y + dt1.U * dv2.y,
										-dt2.U * dv1.z + dt1.U * dv2.z);
			tan *= invda; bn *= invda;

			tangents[tri.Vertex[0]] += tan;
			tangents[tri.Vertex[1]] += tan;
			tangents[tri.Vertex[2]] += tan;
			binormals[tri.Vertex[0]] += bn;
			binormals[tri.Vertex[1]] += bn;
			binormals[tri.Vertex[2]] += bn;
			normals[tri.Vertex[0]] += m_pModel->Normals[tri.Vertex[0]];
			normals[tri.Vertex[1]] += m_pModel->Normals[tri.Vertex[1]];
			normals[tri.Vertex[2]] += m_pModel->Normals[tri.Vertex[2]];
		}
	}

	// averaging & orthogonalize
	for (UINT i = 1; i <= numVertices; i++) {
		ObjTangent& tan = tangents[i];
		ObjBinormal& bn = binormals[i];
		ObjNormal& n = normals[i];
		n = n.normalize();

		// Gram-Schmidt orthogonalize
		ObjTangent tangent = (tan - n * (n * tan)).normalize();
		bn = cross(n, tangent).normalize();
		if ((cross(n, tan) * bn) < 0.0f) bn = -bn;
		tan = tangent;
	}
}

void MeshRenderable::computeNormalMaps(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext) {
	for (auto& texPair : m_vpBumpMaps) {
		ID3D11ShaderResourceView* pTSRV = texPair.second;
		ID3D11Texture2D* pBumpMap = nullptr;
		pTSRV->GetResource((ID3D11Resource**)&pBumpMap);
		D3D11_TEXTURE2D_DESC desc;
		pBumpMap->GetDesc(&desc);
		pBumpMap->Release();
		if (desc.Format != BumpTexFormat)
			checkFailure(E_UNEXPECTED, _T("Unsupported bump map type"));

		UINT bumpTextureSize = desc.Width * desc.Height;
		BTT* pBumpTextureData = new BTT[bumpTextureSize];
		NTT* pNormalMapData = new NTT[bumpTextureSize];

		checkFailure(loadImageData(_T("model\\") + TStringFromANSIString(m_pModel->Materials[texPair.first].BumpMapFileName),
			pBumpTextureData, desc.Width * sizeof(BTT), bumpTextureSize * sizeof(BTT)),
			_T("Failed to load image data from ") + TStringFromANSIString(m_pModel->Materials[texPair.first].BumpMapFileName));

		computeNormalMap(pBumpTextureData, desc.Width, desc.Height, pNormalMapData);
		delete [] pBumpTextureData;

		ID3D11ShaderResourceView* pNormalMapView = nullptr;
		checkFailure(loadTextureFromMemory(pDevice, pDeviceContext, pNormalMapData, desc.Width, desc.Height, NormTexFormat,
			desc.Width * sizeof(NTT), &pNormalMapView),
			_T("Failed to create normal map for material ") + TStringFromANSIString(texPair.first));
		delete [] pNormalMapData;
		m_vpNormalMaps[texPair.first] = pNormalMapView;
	}
}

void MeshRenderable::computeNormalMap(const BTT* pBumpTextureData, UINT width, UINT height, NTT* pNormalMapData) {
	const BTT* p = pBumpTextureData;
	for (UINT y = 1; y < height - 1; y++) {
		for (UINT x = 1; x < width - 1; x++) {
			BTT center = p[y * width + x];
			BTT up = p[(y - 1) * width + x];
			BTT down = p[(y + 1) * width + x];
			BTT left = p[y * width + x - 1];
			BTT right = p[y * width + x + 1];

			FVector vUp = FVector(0, -NormalDistance, (float)((int)up - (int)center) / BTTUpper);
			FVector vDown = FVector(0, NormalDistance, (float)((int)down - (int)center) / BTTUpper);
			FVector vLeft = FVector(-NormalDistance, 0, (float)((int)left - (int)center) / BTTUpper);
			FVector vRight = FVector(NormalDistance, 0, (float)((int)right - (int)center) / BTTUpper);

			FVector vNorm = cross(vUp, vRight) + cross(vRight, vDown) + cross(vDown, vLeft) + cross(vLeft, vUp);
			pNormalMapData[y * width + x] = NTT(vNorm.x / vNorm.z, vNorm.y / vNorm.z);
		}
	}
	for (UINT y = 0; y < height; y++) {
		pNormalMapData[y * width] = pNormalMapData[y * width + width - 1] = XMFLOAT2(0.0, 0.0);
	}
	for (UINT x = 0; x < width; x++) {
		pNormalMapData[x] = pNormalMapData[(height - 1) * width + x] = XMFLOAT2(0.0, 0.0);
	}
}

void MeshRenderable::render(ID3D11DeviceContext* pDeviceContext, IRenderer* pRenderer, const Camera& pCamera) {
	// Set vertex buffer
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	
	// The draw calls
	XMMATRIX matWorld = getWorldMatrix();
	pRenderer->setWorldMatrix(matWorld);
	// Estimate the scaled bump multiplier
	float fBumpMultiplierScale = XMVectorGetX(XMVector4Length(XMVector4Transform(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), matWorld)));
	//TRACE(_T("[MeshRenderable] fBumpMultiplierScale = %.3f\n"), fBumpMultiplierScale);
	UINT nTriDrawn = 0;
	for (const ObjPart& part : m_pModel->Parts) {
		auto iterMtl = m_pModel->Materials.find(part.MaterialName);
		if (m_pModel->Materials.end() != iterMtl) {
			const ObjMaterial& omt = iterMtl->second;
			Material mt(Color(omt.Ambient[0], omt.Ambient[1], omt.Ambient[2], 1.0f),
						Color(omt.Diffuse[0], omt.Diffuse[1], omt.Diffuse[2], 1.0f),
						Color(omt.Specular[0], omt.Specular[1], omt.Specular[2], 1.0f),
						Color(omt.Emission[0], omt.Emission[1], omt.Emission[2], 1.0f),
						omt.Shininess, fBumpMultiplierScale * omt.BumpMultiplier);
			pRenderer->setMaterial(mt);

			auto iterTex = m_vpTextures.find(part.MaterialName);
			if (iterTex != m_vpTextures.end()) {
				pRenderer->useTexture(m_pSamplerState, iterTex->second);
			} else {
				pRenderer->usePlaceholderTexture();
			}
			auto iterBump = m_vpBumpMaps.find(part.MaterialName);
			if (iterBump != m_vpBumpMaps.end()) {
				pRenderer->useBumpMap(m_pSamplerState, iterBump->second);
			} else {
				pRenderer->usePlaceholderBumpMap();
			}
			auto iterNormal = m_vpNormalMaps.find(part.MaterialName);
			if (iterNormal != m_vpNormalMaps.end()) {
				pRenderer->useNormalMap(m_pSamplerState, iterNormal->second);
			} else {
				pRenderer->usePlaceholderNormalMap();
			}
		} else {
			pRenderer->setMaterial(Material::White);
			pRenderer->usePlaceholderTexture();
			pRenderer->usePlaceholderBumpMap();
			pRenderer->usePlaceholderNormalMap();
		}

		pDeviceContext->Draw(3 * (part.TriIdxMax - part.TriIdxMin), 3 * nTriDrawn);
		nTriDrawn += part.TriIdxMax - part.TriIdxMin;
	}
}

void MeshRenderable::cleanup(IRenderer* pRenderer) {
	IUnknown** ppUnknowns[] = {
		(IUnknown**)&m_pVertexBuffer,
		(IUnknown**)&m_pIndexBuffer,
		(IUnknown**)&m_pSamplerState
	};

	releaseCOMObjs(ppUnknowns);

	for (auto& pTexturePair : m_vpTextures) {
		pTexturePair.second->Release();
	}
	m_vpTextures.clear();

	for (auto& pBumpMapPair : m_vpBumpMaps) {
		pBumpMapPair.second->Release();
	}
	m_vpBumpMaps.clear();

	for (auto& pNormalMapPair : m_vpNormalMaps) {
		pNormalMapPair.second->Release();
	}
	m_vpNormalMaps.clear();
}
