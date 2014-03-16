#pragma once

#include "ObjLoader.h"

namespace Utils {

	void computeNormals(ObjModel* pModel);
	void computeTangentSpace(ObjModel* pModel);
	void duplicateVerticesForDifferentTexCoords(ObjModel* pModel);
	// Remove part of the model in the hemispace Ax+By+Cz+D>0
	void removeModelPart(ObjModel* pModel, float A, float B, float C, float D);

} // namespace Utils
