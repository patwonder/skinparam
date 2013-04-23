/**
 * Interface for renderable objects
 * This file is for default implementaions
 */

#include "stdafx.h"

#include "Renderable.h"

using namespace Skin;
using namespace Utils;

XMMATRIX Renderable::getWorldMatrix() const {
	return XMMatrixIdentity();
}

Material Renderable::getMaterial() const {
	return Material::White;
}
