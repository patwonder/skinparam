/**
 * Head renderer
 */

#include "stdafx.h"

#include "Head.h"
#include "Utils/NormalCalc.h"

using namespace Skin;
using namespace Utils;

Head::Head() : MeshRenderable(_T("model\\head.OBJ")) {
	//removeModelPart(m_pModel, 0, -1, 0, -0.15f);
	//FVector vl, vu;
	//getBoundingBox(vl, vu);
	//FVector extent = vu - vl;
	//TStringStream tss;
	//tss << extent.x << " * " << extent.y << " * " << extent.z;
	//MessageBox(AfxGetMainWnd()->m_hWnd, tss.str().c_str(), NULL, MB_OK);
	m_roughness = 0.3f;
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
