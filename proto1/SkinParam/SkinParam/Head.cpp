/**
 * Head renderer
 */

#include "stdafx.h"

#include "Head.h"
#include "Utils/NormalCalc.h"

using namespace Skin;
using namespace Utils;

Head::Head() : MeshRenderable(_T("model\\head.OBJ")) {
	removeModelPart(m_pModel, 0, -1, 0, -0.15f);
}

Head::~Head() {}

XMMATRIX Head::getWorldMatrix() const {
	XMMATRIX mtWorld = XMMatrixScaling(10.0f, 10.0f, 10.0f);
	mtWorld *= XMMatrixRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), (float)(Math::PI / 2));
	mtWorld *= XMMatrixTranslation(0.0f, 0.0f, 0.2f);
	return mtWorld;
}

void Head::render(ID3D11DeviceContext* pDeviceContext, IRenderer* pRenderer, const Camera& pCamera) {
	pRenderer->setTessellationFactor(1.0f, 1.0f, 1.0f, 9.0f);
	MeshRenderable::render(pDeviceContext, pRenderer, pCamera);
}
