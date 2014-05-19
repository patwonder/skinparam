
/*
    Copyright(c) 2013-2014 Yifan Wu.

    This file is part of SkinParam.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

#include "NormalCalc.h"
#include <unordered_map>
#include <unordered_set>

void Utils::computeNormals(ObjModel* pModel) {
	// quit if there's already some normals in the model
	// note normals (and vertices, texcoords) begin at index 1
	if (pModel->Normals.size() > 1) return;
	
	// avoid computing different normals for the same vertex
	//removeDuplicateVertices();

	size_t numVertices = pModel->Vertices.size() - 1;

	auto& normals = pModel->Normals;
	normals.resize(pModel->Vertices.size());
	// normals are automatically zeroed when FVectors are constructed

	for (const ObjPart& part : pModel->Parts) {
		for (int idxTri = part.TriIdxMin; idxTri < part.TriIdxMax; idxTri++) {
			const ObjTriangle& tri = pModel->Triangles[idxTri];
			const FVector& v1 = pModel->Vertices[tri.Vertex[0]];
			const FVector& v2 = pModel->Vertices[tri.Vertex[1]];
			const FVector& v3 = pModel->Vertices[tri.Vertex[2]];

			FVector normal = (v2 - v1).cross(v3 - v1);
			normals[tri.Vertex[0]] += normal;
			normals[tri.Vertex[1]] += normal;
			normals[tri.Vertex[2]] += normal;
		}
	}

	// calculate average normal
	for (size_t i = 1; i <= numVertices; i++) {
		if (normals[i])
			normals[i] = normals[i].normalize();
	}

	// apply normals to faces
	for (const ObjPart& part : pModel->Parts) {
		for (int idxTri = part.TriIdxMin; idxTri < part.TriIdxMax; idxTri++) {
			ObjTriangle& tri = pModel->Triangles[idxTri];
			for (int j = 0; j < 3; j++) {
				tri.Normal[j] = tri.Vertex[j];
			}
		}
	}
}

void Utils::computeTangentSpace(ObjModel* pModel) {
	// quit if there's already some tangents/binormals in the model
	// note tangents/binormals (and vertices, texcoords) begin at index 1
	if (pModel->Tangents.size() > 1 || pModel->Binormals.size() > 1) return;

	size_t numVertices = pModel->Vertices.size() - 1;
	
	// initialization
	std::vector<ObjNormal> normals;
	auto& tangents = pModel->Tangents;
	auto& binormals = pModel->Binormals;
	normals.resize(pModel->Vertices.size());
	tangents.resize(pModel->Vertices.size());
	binormals.resize(pModel->Vertices.size());
	// already zeroed when default constructors are called

	// See http://www.terathon.com/code/binormal.html
	for (const ObjPart& part : pModel->Parts) {
		for (int idxTri = part.TriIdxMin; idxTri < part.TriIdxMax; idxTri++) {
			const ObjTriangle& tri = pModel->Triangles[idxTri];
			const ObjTexCoord& t0 = pModel->TexCoords[tri.TexCoord[0]];
			const ObjTexCoord& t1 = pModel->TexCoords[tri.TexCoord[1]];
			const ObjTexCoord& t2 = pModel->TexCoords[tri.TexCoord[2]];
			const ObjVertex& v0 = pModel->Vertices[tri.Vertex[0]];
			const ObjVertex& v1 = pModel->Vertices[tri.Vertex[1]];
			const ObjVertex& v2 = pModel->Vertices[tri.Vertex[2]];

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
			normals[tri.Vertex[0]] += pModel->Normals[tri.Normal[0]];
			normals[tri.Vertex[1]] += pModel->Normals[tri.Normal[1]];
			normals[tri.Vertex[2]] += pModel->Normals[tri.Normal[2]];
		}
	}

	// averaging & orthogonalize
	for (size_t i = 1; i <= numVertices; i++) {
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

	// apply tangents & binormals to faces
	for (const ObjPart& part : pModel->Parts) {
		for (int idxTri = part.TriIdxMin; idxTri < part.TriIdxMax; idxTri++) {
			ObjTriangle& tri = pModel->Triangles[idxTri];
			for (int j = 0; j < 3; j++) {
				tri.Tangent[j] = tri.Binormal[j] = tri.Vertex[j];
			}
		}
	}
}

void Utils::duplicateVerticesForDifferentTexCoords(ObjModel* pModel) {
	for (const ObjPart& part : pModel->Parts) {
		std::unordered_map<size_t, size_t> mapVertexToTexCoord;
		std::unordered_map<size_t, std::unordered_set<size_t> > mapVertexDuplicates;

		for (int idxTri = part.TriIdxMin; idxTri < part.TriIdxMax; idxTri++) {
			ObjTriangle& tri = pModel->Triangles[idxTri];
			for (int j = 0; j < 3; j++) {
				auto iter = mapVertexToTexCoord.find(tri.Vertex[j]);
				if (iter != mapVertexToTexCoord.end()) {
					if (iter->second != tri.TexCoord[j]) {
						const ObjTexCoord& t1 = pModel->TexCoords[iter->second];
						const ObjTexCoord& t2 = pModel->TexCoords[tri.TexCoord[j]];
						if (t1.U != t2.U || t1.V != t2.V) {
							// contour vertex, look for duplicates
							auto& setDuplicates = mapVertexDuplicates[tri.Vertex[j]];
							for (size_t dupId : setDuplicates) {
								size_t texId = mapVertexToTexCoord[dupId];
								const ObjTexCoord& t3 = pModel->TexCoords[texId];
								if (texId == tri.TexCoord[j] || (t2.U == t3.U && t2.V == t3.V)) {
									// already duplicated, assign index to triangle vertex
									tri.Vertex[j] = dupId;
									goto NextVertex;
								}
							}
							// no duplicates yet, create one
							size_t dupId = pModel->Vertices.size();
							pModel->Vertices.push_back(pModel->Vertices[tri.Vertex[j]]);
							// record the duplicate
							mapVertexDuplicates[tri.Vertex[j]].insert(dupId);
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
	}
}

// Remove part of the model in the hemispace Ax+By+Cz+D>0
void Utils::removeModelPart(ObjModel* pModel, float A, float B, float C, float D) {
	int triIdxBase = 0;
	std::vector<ObjTriangle> newTriangles;
	for (ObjPart& part : pModel->Parts) {
		for (int idxTri = part.TriIdxMin; idxTri < part.TriIdxMax; idxTri++) {
			const ObjTriangle& tri = pModel->Triangles[idxTri];
			bool keep = true;
			for (int j = 0; j < 3; j++) {
				const ObjVertex& v = pModel->Vertices[tri.Vertex[j]];
				if (A * v.x + B * v.y + C * v.z + D > 0) {
					keep = false; break;
				}
			}
			if (keep) {
				newTriangles.push_back(tri);
			}
		}
		part.TriIdxMin = triIdxBase;
		part.TriIdxMax = triIdxBase = newTriangles.size();
	}
	pModel->Triangles.swap(newTriangles);
}
