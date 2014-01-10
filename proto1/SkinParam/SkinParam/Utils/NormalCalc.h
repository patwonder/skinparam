#pragma once

#include "ObjLoader.h"

namespace Utils {

	void computeNormals(ObjModel* pModel);
	void computeTangentSpace(ObjModel* pModel);
	void duplicateVerticesForDifferentTexCoords(ObjModel* pModel);
} // namespace Utils
