/**
 * Renderable that capable of rendering meshes
 */

#include "stdafx.h"

#include "MeshRenderable.h"
#include "ObjLoader.h"
#include "D3DHelper.h"
#include "DirectXTex\DDSTextureLoader\DDSTextureLoader.h"
#include <unordered_set>
#include <tuple>

using namespace Skin;
using namespace Utils;
using namespace D3DHelper;

const float MeshRenderable::NormalDistance = 0.04f;
const WICPixelFormatGUID MeshRenderable::BumpTexWICFormat = GUID_WICPixelFormat16bppGray;

MeshRenderable::BumpMapSampleData::BumpMapSampleData() {
	pData = nullptr; width = 0; height = 0;
}

MeshRenderable::BumpMapSampleData::~BumpMapSampleData() {
	freeImageData(pData);
}

MeshRenderable::MeshRenderable(const TString& strObjFilePath) {
	m_pModel = nullptr;

	std::string path = ANSIStringFromTString(strObjFilePath);
	ObjLoader loader(path);

	m_pModel = loader.ReturnObj();
	computeNormals();
	computeBoundingSphere();
	detectContourVertices();
	computeTangentSpace();
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
				auto& iter = m_mapContourBump.find(tri.Vertex[j]);
				if (iter != m_mapContourBump.end()) {
					v.bumpOverride = XMFLOAT2(iter->second, 1.0f);
				} else {
					v.bumpOverride = XMFLOAT2(0.5f, 0.0f);
				}
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
			CComPtr<ID3D11ShaderResourceView> pTexture = nullptr;
			bool dds = isDDSFile(TStringFromANSIString(objMt.TextureFileName));
			DirectX::DDS_ALPHA_MODE alphaMode = DirectX::DDS_ALPHA_MODE_STRAIGHT;
			// use srgb for albedo textures
			checkFailure(dds 
				? loadDDSTextureEx(pDevice, _T("model\\") + TStringFromANSIString(objMt.TextureFileName),
				  0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, true, &pTexture, &alphaMode)
				: loadTextureEx(pDevice, pRenderer->getDeviceContext(), _T("model\\") + TStringFromANSIString(objMt.TextureFileName),
				  0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, true, &pTexture),
				_T("Unable to load texture ") + TStringFromANSIString(objMt.TextureFileName));
			//ASSERT(alphaMode == DirectX::DDS_ALPHA_MODE_STRAIGHT);
			m_vpTextures[objMtPair.first] = pTexture;
		}
		if (objMt.BumpMapFileName.length()) {
			CComPtr<ID3D11ShaderResourceView> pBumpMap = nullptr;
			bool dds = isDDSFile(TStringFromANSIString(objMt.BumpMapFileName));
			DirectX::DDS_ALPHA_MODE alphaMode = DirectX::DDS_ALPHA_MODE_UNKNOWN;
			checkFailure(dds
				? loadDDSTexture(pDevice, _T("model\\") + TStringFromANSIString(objMt.BumpMapFileName), &pBumpMap, &alphaMode)
				: loadTexture(pDevice, pRenderer->getDeviceContext(), _T("model\\") + TStringFromANSIString(objMt.BumpMapFileName), &pBumpMap),
				  _T("Unable to load bump map ") + TStringFromANSIString(objMt.BumpMapFileName));
			//ASSERT(alphaMode == DirectX::DDS_ALPHA_MODE_UNKNOWN);
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

void MeshRenderable::computeBoundingSphere() {
	m_vCenter = FVector::ZERO;
	for (UINT i = 1; i < m_pModel->Vertices.size(); i++) {
		const ObjVertex& v = m_pModel->Vertices[i];
		m_vCenter += v;
	}
	if (m_pModel->Vertices.size() > 1)
		m_vCenter /= (float)(m_pModel->Vertices.size() - 1);

	m_fBoundingSphereRadius = 0.0f;
	for (UINT i = 1; i < m_pModel->Vertices.size(); i++) {
		const ObjVertex& v = m_pModel->Vertices[i];
		float l = (v - m_vCenter).length();
		if (l > m_fBoundingSphereRadius)
			m_fBoundingSphereRadius = l;
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
			float invda = 1.0f / (dt1.U * dt2.V - dt1.V * dt2.U);
			ObjTangent tan = ObjTangent(dt2.V * dv1.x - dt1.V * dv2.x, 
										 dt2.V * dv1.y - dt1.V * dv2.y,
										 dt2.V * dv1.z - dt1.V * dv2.z);
			ObjBinormal bn = ObjBinormal(-dt2.U * dv1.x + dt1.U * dv2.x,
										-dt2.U * dv1.y + dt1.U * dv2.y,
										-dt2.U * dv1.z + dt1.U * dv2.z);
			if (invda != invda || isInfinite(invda)) continue;
			tan *= invda; bn *= invda;

			tangents[tri.Vertex[0]] += tan;
			tangents[tri.Vertex[1]] += tan;
			tangents[tri.Vertex[2]] += tan;
			binormals[tri.Vertex[0]] += bn;
			binormals[tri.Vertex[1]] += bn;
			binormals[tri.Vertex[2]] += bn;
			normals[tri.Vertex[0]] += m_pModel->Normals[tri.Normal[0]];
			normals[tri.Vertex[1]] += m_pModel->Normals[tri.Normal[1]];
			normals[tri.Vertex[2]] += m_pModel->Normals[tri.Normal[2]];
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
		bool illHanded = (cross(n, tan) * bn) < 0.0f;
		bn = cross(n, tangent).normalize();
		if (illHanded) bn = -bn;
		tan = tangent;
	}
}

void MeshRenderable::computeNormalMaps(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext) {
	for (auto& texPair : m_vpBumpMaps) {
		BTT* pBumpTextureData = nullptr;
		UINT width, height;
		WICPixelFormatGUID pixelFormatGUID;
		TString bumpFileName = TStringFromANSIString(m_pModel->Materials[texPair.first].BumpMapFileName);
		checkFailure(loadImageData(_T("model\\") + bumpFileName, (void**)&pBumpTextureData, &width, &height, &pixelFormatGUID),
			_T("Failed to load image data from ") + bumpFileName);

		if (pixelFormatGUID != BumpTexWICFormat)
			checkFailure(E_UNEXPECTED, _T("Unsupported bump map type"));

		NTT* pNormalMapData = new NTT[width * height];

		computeNormalMap(pBumpTextureData, width, height, pNormalMapData);
		delete [] pBumpTextureData;

		CComPtr<ID3D11ShaderResourceView> pNormalMapView;
		checkFailure(loadTextureFromMemory(pDevice, pDeviceContext, pNormalMapData, width, height, NormTexFormat,
			width * sizeof(NTT), &pNormalMapView),
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

void MeshRenderable::detectContourVerticesForPart(const ObjPart& part) {
	// prepare bump map for sampling
	BumpMapSampleData sd;

	// First step, detect vertices with different texcoords in different triangles

	std::unordered_map<UINT, UINT> mapVertexToTexCoord;
	std::unordered_map<UINT, std::unordered_set<UINT> > mapVertexDuplicates;

	for (int idxTri = part.TriIdxMin; idxTri < part.TriIdxMax; idxTri++) {
		ObjTriangle& tri = m_pModel->Triangles[idxTri];
		for (int j = 0; j < 3; j++) {
			auto iter = mapVertexToTexCoord.find(tri.Vertex[j]);
			if (iter != mapVertexToTexCoord.end()) {
				if (iter->second != tri.TexCoord[j]) {
					const ObjTexCoord& t1 = m_pModel->TexCoords[iter->second];
					const ObjTexCoord& t2 = m_pModel->TexCoords[tri.TexCoord[j]];
					if (t1.U != t2.U || t1.V != t2.V) {
						// contour vertex, look for duplicates
						auto& setDuplicates = mapVertexDuplicates[tri.Vertex[j]];
						for (UINT dupId : setDuplicates) {
							UINT texId = mapVertexToTexCoord[dupId];
							const ObjTexCoord& t3 = m_pModel->TexCoords[texId];
							if (texId == tri.TexCoord[j] || (t2.U == t3.U && t2.V == t3.V)) {
								// already duplicated, assign index to triangle vertex
								tri.Vertex[j] = dupId;
								goto NextVertex;
							}
						}
						// no duplicates yet, create one
						UINT dupId = m_pModel->Vertices.size();
						m_pModel->Vertices.push_back(m_pModel->Vertices[tri.Vertex[j]]);
						// record the duplicate
						mapVertexDuplicates[tri.Vertex[j]].insert(dupId);
						float bumpSample = sampleBumpMap(&sd.pData, &sd.width, &sd.height, t1, part.MaterialName);
						m_mapContourBump[tri.Vertex[j]] = bumpSample;
						m_mapContourBump[dupId] = bumpSample;
						// assign duplicate index
						tri.Vertex[j] = dupId;
						mapVertexToTexCoord[dupId] = tri.TexCoord[j];
					}
				}
			} else {
				mapVertexToTexCoord[tri.Vertex[j]] = tri.TexCoord[j];
			}
NextVertex:
			;
		}
	}

	// Second step, detect vertices adjacent to contour vertices with different texcoords
	// mark them also as contour vertices to avoid aliasing through tessellation

	// mapping duplicate vertices back to its origin
	std::unordered_map<UINT, UINT> mapDuplicatedToOrigin;
	for (const auto& pair : mapVertexDuplicates) {
		mapDuplicatedToOrigin[pair.first] = pair.first;
		for (UINT dupId : pair.second) {
			mapDuplicatedToOrigin[dupId] = pair.first;
		}
	}

	// build adjacency map
	std::unordered_map<UINT, std::unordered_set<UINT> > mapVertexToAdjacentContour;
	for (int idxTri = part.TriIdxMin; idxTri < part.TriIdxMax; idxTri++) {
		ObjTriangle& tri = m_pModel->Triangles[idxTri];
		// for each edge...
		for (int j = 0; j < 3; j++) {
			int v1 = (j + 1) % 3, v2 = (j + 2) % 3;
			bool bContour1 = m_mapContourBump.find(tri.Vertex[v1]) != m_mapContourBump.end();
			bool bContour2 = m_mapContourBump.find(tri.Vertex[v2]) != m_mapContourBump.end();
			if (bContour1 == bContour2) // two contours or no contours
				continue;
			// Add to adjacency map
			if (bContour1)
				mapVertexToAdjacentContour[tri.Vertex[v2]].insert(tri.Vertex[v1]);
			else
				mapVertexToAdjacentContour[tri.Vertex[v1]].insert(tri.Vertex[v2]);
		}
	}

	// seach for vertices adjacent to same contour vertex with different texcoords
	for (const auto& pair : mapVertexToAdjacentContour) {
		std::unordered_set<UINT> setAdjacentContourOrigins;
		for (UINT contourId : pair.second) {
			UINT oContourId = mapDuplicatedToOrigin[contourId];
			if (setAdjacentContourOrigins.find(oContourId) != setAdjacentContourOrigins.end()) {
				// Found the vertex, add it to set of contour vertices
				const ObjTexCoord& texCoord = m_pModel->TexCoords[mapVertexToTexCoord[pair.first]];
				float bumpSample = sampleBumpMap(&sd.pData, &sd.width, &sd.height, texCoord, part.MaterialName);
				m_mapContourBump[pair.first] = bumpSample;

				break;
			} else {
				setAdjacentContourOrigins.insert(oContourId);
			}
		}
	}
}

void MeshRenderable::detectContourVertices() {
	for (const ObjPart& part : m_pModel->Parts) {
		detectContourVerticesForPart(part);
	}
}

float MeshRenderable::sampleBumpMap(BTT** ppData, UINT* pWidth, UINT* pHeight, ObjTexCoord texCoord, const std::string& materialName) {
	if (!*ppData) {
		TString bumpFileName = TStringFromANSIString(m_pModel->Materials[materialName].BumpMapFileName);
		WICPixelFormatGUID pixelFormatGUID;
		checkFailure(loadImageData(_T("model\\") + bumpFileName, (void**)ppData, pWidth, pHeight, &pixelFormatGUID),
			_T("Failed to load bump texture data for sampling from ") + bumpFileName);

		if (pixelFormatGUID != BumpTexWICFormat)
			checkFailure(E_UNEXPECTED, _T("Unsupported bump map type"));
	}
	// use bilinear sampling
	float pixelWidth = 1.0f / (*pWidth);
	float x = Math::clampValue(texCoord.U * (*pWidth) - 0.5f, 0.0f, (*pWidth) - 1.0f);
	float y = Math::clampValue(texCoord.V * (*pHeight) - 0.5f, 0.0f, (*pHeight) - 1.0f);
	UINT ix = (UINT)x, iy = (UINT)y;
	if (ix == (*pWidth - 1)) ix--;
	if (iy == (*pHeight - 1)) iy--;
	float fracx = x - ix, fracy = y - iy;
	float samples[4];
	samples[0] = (float)(*ppData)[iy * (*pWidth) + ix] / BTTUpper;
	samples[1] = (float)(*ppData)[iy * (*pWidth) + ix + 1] / BTTUpper;
	samples[2] = (float)(*ppData)[(iy + 1) * (*pWidth) + ix] / BTTUpper;
	samples[3] = (float)(*ppData)[(iy + 1) * (*pWidth) + ix + 1] / BTTUpper;
	return Math::lerp(Math::lerp(samples[0], samples[1], fracx), Math::lerp(samples[2], samples[3], fracx), fracy);
}

float MeshRenderable::getBumpMultiplierScale(const XMMATRIX& matWorld) {
	return XMVectorGetX(XMVector4Length(XMVector4Transform(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), matWorld)));
}

void MeshRenderable::render(ID3D11DeviceContext* pDeviceContext, IRenderer* pRenderer, const Camera& pCamera) {
	// Set vertex buffer
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer.p, &stride, &offset);
	
	// The draw calls
	XMMATRIX matWorld = getWorldMatrix();
	pRenderer->setWorldMatrix(matWorld);
	// Estimate the scaled bump multiplier
	float fBumpMultiplierScale = getBumpMultiplierScale(matWorld);
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

}

void MeshRenderable::getBoundingSphere(FVector& oVecCenter, float& oRadius) const {
	// estimate translation and scaling using world matrix
	XMMATRIX matWorld = getWorldMatrix();
	oRadius = m_fBoundingSphereRadius * getBumpMultiplierScale(matWorld);
	XMVECTOR vecTrans = XMVector4Transform(XMVectorSet(m_vCenter.x, m_vCenter.y, m_vCenter.z, 1.0f), matWorld);
	oVecCenter = FVector(XMVectorGetX(vecTrans), XMVectorGetY(vecTrans), XMVectorGetZ(vecTrans));
}
