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
