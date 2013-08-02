/*
 * Interface for drawable objects
 */

#include "stdafx.h"

#include "Drawable.h"

using namespace RLSkin;
using namespace Utils;

void Drawable::getBoundingSphere(FVector& oVecCenter, float& oRadius) const {
	oVecCenter = FVector::ZERO;
	oRadius = 0.0f;
}
