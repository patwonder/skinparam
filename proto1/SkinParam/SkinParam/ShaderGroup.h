/**
 * Shaders that are tied together in one rendering pass
 */

#pragma once

#include "TString.h"

namespace Skin {
	class ShaderGroup {
	private:
		ID3D11InputLayout* pInputLayout;
		ID3D11VertexShader* pVertexShader;
		ID3D11PixelShader* pPixelShader;
	public:
		ShaderGroup(ID3D11Device* pDevice, const Utils::TString& strFileName, 
			D3D11_INPUT_ELEMENT_DESC aLayoutDesc[], UINT numLayoutDesc, // Input Layout
			const char* szVSEntryPoint, // Vertex Shader
			const char* szPSEntryPoint); // Pixel Shader
		~ShaderGroup();

		void use(ID3D11DeviceContext* pDeviceContext) const;
	};
};
