/**
 * Head renderer
 */

#include "stdafx.h"

#include "Head.h"

using namespace Skin;
using namespace Utils;

Head::Head() : MeshRenderable(_T("model\\head.OBJ")) {
}

Head::~Head() {}

XMMATRIX Head::getWorldMatrix() const {
	XMMATRIX mtWorld = XMMatrixScaling(10.0f, 10.0f, 10.0f);
	mtWorld *= XMMatrixRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), (float)(Math::PI / 2));
	mtWorld *= XMMatrixTranslation(0.0f, 0.0f, 0.2f);
	return mtWorld;
}

void Head::render(ID3D11DeviceContext* pDeviceContext, IRenderer* pRenderer, const Camera& pCamera) {
	pRenderer->setTessellationFactor(1.0f, 1.0f, 1.0f, 30.0f);
	MeshRenderable::render(pDeviceContext, pRenderer, pCamera);
}
