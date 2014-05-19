
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
 * Shaders that are tied together in one rendering pass
 */

#include "stdafx.h"

#include "ShaderGroup.h"
#include "D3DHelper.h"

using namespace Skin;
using namespace Utils;
using namespace D3DHelper;

ShaderGroup::ShaderGroup(ID3D11Device* pDevice, const Utils::TString& strFileName, 
	D3D11_INPUT_ELEMENT_DESC aLayoutDesc[], UINT numLayoutDesc, // Input Layout
	const char* szVSEntryPoint, // Vertex Shader
	const char* szHSEntryPoint, // Hull Shader
	const char* szDSEntryPoint, // Domain Shader
	const char* szPSEntryPoint) // Pixel Shader
	: pInputLayout(nullptr), pVertexShader(nullptr), pHullShader(nullptr),
	  pDomainShader(nullptr), pPixelShader(nullptr)
{
	checkFailure(loadVertexShaderAndLayout(pDevice, strFileName, szVSEntryPoint, aLayoutDesc, numLayoutDesc,
		&pVertexShader, &pInputLayout), _T("Failed to load vertex shader"));

	if (szHSEntryPoint && strlen(szHSEntryPoint)) {
		if (!szDSEntryPoint || !strlen(szDSEntryPoint))
			checkFailure(E_INVALIDARG, _T("Missing domain shader for hull shader"));

		checkFailure(loadHullShader(pDevice, strFileName, szHSEntryPoint, &pHullShader),
			_T("Failed to load hull shader"));

		checkFailure(loadDomainShader(pDevice, strFileName, szDSEntryPoint, &pDomainShader),
			_T("Failed to load domain shader"));

	} else if (szDSEntryPoint && strlen(szDSEntryPoint)) {
		checkFailure(E_INVALIDARG, _T("Missing hull shader for domain shader"));
	}

	checkFailure(loadPixelShader(pDevice, strFileName, szPSEntryPoint, &pPixelShader),
		_T("Failed to load pixel shader"));
}

ShaderGroup::~ShaderGroup()
{
	IUnknown** ppUnknowns[] = {
		(IUnknown**)&pInputLayout,
		(IUnknown**)&pVertexShader,
		(IUnknown**)&pHullShader,
		(IUnknown**)&pDomainShader,
		(IUnknown**)&pPixelShader
	};
	releaseCOMObjs(ppUnknowns);
}

void ShaderGroup::use(ID3D11DeviceContext* pDeviceContext) const {
	pDeviceContext->IASetInputLayout(pInputLayout);
	pDeviceContext->VSSetShader(pVertexShader, NULL, 0);
	if (pHullShader) {
		pDeviceContext->HSSetShader(pHullShader, NULL, 0);
		pDeviceContext->DSSetShader(pDomainShader, NULL, 0);
	} else {
		pDeviceContext->HSSetShader(NULL, NULL, 0);
		pDeviceContext->DSSetShader(NULL, NULL, 0);
	}
	pDeviceContext->PSSetShader(pPixelShader, NULL, 0);
}
