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
	const char* szPSEntryPoint) // Pixel Shader
	: pInputLayout(nullptr), pVertexShader(nullptr), pPixelShader(nullptr)
{
	checkFailure(loadVertexShaderAndLayout(pDevice, strFileName, szVSEntryPoint, aLayoutDesc, numLayoutDesc,
		&pVertexShader, &pInputLayout), _T("Failed to load vertex shader"));

	checkFailure(loadPixelShader(pDevice, strFileName, szPSEntryPoint, &pPixelShader),
		_T("Failed to load pixel shader"));
}

ShaderGroup::~ShaderGroup()
{
	IUnknown** ppUnknowns[] = {
		(IUnknown**)&pInputLayout,
		(IUnknown**)&pVertexShader,
		(IUnknown**)&pPixelShader
	};
	releaseCOMObjs(ppUnknowns);
}

void ShaderGroup::use(ID3D11DeviceContext* pDeviceContext) const {
	pDeviceContext->IASetInputLayout(pInputLayout);
	pDeviceContext->VSSetShader(pVertexShader, NULL, 0);
	pDeviceContext->PSSetShader(pPixelShader, NULL, 0);
}
