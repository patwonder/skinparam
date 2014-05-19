
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
