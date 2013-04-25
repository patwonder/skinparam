/**
 * Renderable that capable of rendering meshes
 */

#include "stdafx.h"

#include "MeshRenderable.h"
#include "ObjLoader.h"
#include "D3DHelper.h"

using namespace Skin;
using namespace Utils;
using namespace D3DHelper;

MeshRenderable::MeshRenderable(const TString& strObjFilePath)
	: m_pVertexBuffer(nullptr), m_pIndexBuffer(nullptr), m_pSamplerState(nullptr)
{
	m_pModel = nullptr;

	std::string path = ANSIStringFromTString(strObjFilePath);
	ObjLoader loader(path);

	m_pModel = loader.ReturnObj();
	computeNormals();
}

MeshRenderable::~MeshRenderable() {
	delete m_pModel;
}

void MeshRenderable::init(ID3D11Device* pDevice) {
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
				v.texCoord = XMFLOAT2(&m_pModel->TexCoords[tri.TexCoord[j]].U);
				i++;
			}
		}
	}

	checkFailure(createVertexBuffer(pDevice, vertices, numVertices, &m_pVertexBuffer),
		_T("Failed to create vertex buffer for mesh"));

	delete[] vertices;

	// Create sampler state
	checkFailure(createSamplerState(pDevice, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, &m_pSamplerState),
		_T("Failed to create sampler state"));

	// Create textures for each material
	for (const auto& objMtPair : m_pModel->Materials) {
		const ObjMaterial& objMt = objMtPair.second;
		if (objMt.TextureFileName.length()) {
			ID3D11ShaderResourceView* pTexture = nullptr;
			loadTexture(pDevice, _T("model\\") + TStringFromANSIString(objMt.TextureFileName), &pTexture);
			m_vpTextures[objMtPair.first] = pTexture;
		}
	}
}

void MeshRenderable::computeNormals() {
	// quit if there's already some normals in the model
	// note normals (and vertices, texcoords) begin at index 1
	if (m_pModel->Normals.size() > 1) return;

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

			FVector normal = (v2 - v1).cross(v3 - v1).normalize();
			normals[tri.Vertex[0]] += normal;
			normals[tri.Vertex[1]] += normal;
			normals[tri.Vertex[2]] += normal;
		}
	}

	// calculate average normal
	for (UINT i = 0; i < numVertices; i++) {
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

void MeshRenderable::render(ID3D11DeviceContext* pDeviceContext, IRenderer* pRenderer, const Camera& pCamera) {
	// Set vertex buffer
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	
	// Set primitive topology
    pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// The draw calls
	pRenderer->setWorldMatrix(getWorldMatrix());

	UINT nTriDrawn = 0;
	for (const ObjPart& part : m_pModel->Parts) {
		auto iterMtl = m_pModel->Materials.find(part.MaterialName);
		if (m_pModel->Materials.end() != iterMtl) {
			const ObjMaterial& omt = iterMtl->second;
			Material mt(Color(omt.Ambient[0], omt.Ambient[1], omt.Ambient[2], 1.0f),
						Color(omt.Diffuse[0], omt.Diffuse[1], omt.Diffuse[2], 1.0f),
						Color(omt.Specular[0], omt.Specular[1], omt.Specular[2], 1.0f),
						Color(omt.Emission[0], omt.Emission[1], omt.Emission[2], 1.0f),
						omt.Shininess);
			pRenderer->setMaterial(mt);

			auto iterTex = m_vpTextures.find(part.MaterialName);
			if (iterTex != m_vpTextures.end()) {
				ID3D11ShaderResourceView* pTexture = iterTex->second;
				pDeviceContext->PSSetSamplers(0, 1, &m_pSamplerState);
				pDeviceContext->PSSetShaderResources(0, 1, &pTexture);
			} else {
				pRenderer->usePlaceholderTexture();
			}
		} else {
			pRenderer->setMaterial(Material::White);
			pRenderer->usePlaceholderTexture();
		}

		pDeviceContext->Draw(3 * (part.TriIdxMax - part.TriIdxMin), 3 * nTriDrawn);
		nTriDrawn += part.TriIdxMax - part.TriIdxMin;
	}
}

void MeshRenderable::cleanup() {
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
}
