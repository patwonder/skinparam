/**
 * D3D helper functions
 */

#pragma once

#include "TString.h"

namespace Skin {
	namespace D3DHelper {
		// error reporting
		void checkFailure(HRESULT hr, const Utils::TString& prompt);
		// shaders
		HRESULT compileShader(const Utils::TString& strFileName, const char* szEntryPoint, const char* szShaderModel, ID3DBlob** ppBlobOut);
		HRESULT loadVertexShaderAndLayout(ID3D11Device* pDevice,
			const Utils::TString& strFileName, const char* szEntryPoint,
			D3D11_INPUT_ELEMENT_DESC aLayoutDesc[], UINT numLayoutDesc,
			ID3D11VertexShader** ppVertexShader, ID3D11InputLayout** ppInputLayout);
		HRESULT loadPixelShader(ID3D11Device* pDevice,
			const Utils::TString& strFileName, const char* szEntryPoint,
			ID3D11PixelShader** ppPixelShader);
		// buffers
		HRESULT createBuffer(ID3D11Device* pDevice, UINT byteWidth, UINT bindFlags, void* pData, ID3D11Buffer** ppVertexBuffer); // for internal use
		template <class VertexType>
		HRESULT createVertexBuffer(ID3D11Device* pDevice, VertexType aVertices[], UINT numVertices, ID3D11Buffer** ppVertexBuffer);
		template <class IndexType>
		HRESULT createIndexBuffer(ID3D11Device* pDevice, IndexType aVertices[], UINT numIndices, ID3D11Buffer** ppIndexBuffer);
		template <class ConstantType>
		HRESULT createConstantBuffer(ID3D11Device* pDevice, ConstantType* data, ID3D11Buffer** ppConstantBuffer);
		// house keeping
		template <UINT numUnknowns>
		void releaseCOMObjs(IUnknown** (&appUnknowns)[numUnknowns]);

		// template implementation
		template <class VertexType>
		HRESULT createVertexBuffer(ID3D11Device* pDevice, VertexType aVertices[], UINT numVertices, ID3D11Buffer** ppVertexBuffer) {
			return createBuffer(pDevice, sizeof(VertexType) * numVertices, D3D11_BIND_VERTEX_BUFFER, aVertices, ppVertexBuffer);
		}

		template <class IndexType>
		HRESULT createIndexBuffer(ID3D11Device* pDevice, IndexType aIndices[], UINT numIndices, ID3D11Buffer** ppIndexBuffer) {
			return createBuffer(pDevice, sizeof(IndexType) * numIndices, D3D11_BIND_INDEX_BUFFER, aIndices, ppIndexBuffer);
		}

		template <class ConstantType>
		static HRESULT createConstantBuffer(ID3D11Device* pDevice, ConstantType* data, ID3D11Buffer** ppConstantBuffer) {
			return createBuffer(pDevice, sizeof(ConstantType), D3D11_BIND_CONSTANT_BUFFER, &data, ppConstantBuffer);
		}
		template <UINT numUnknowns>
		void D3DHelper::releaseCOMObjs(IUnknown** (&appUnknowns)[numUnknowns]) {
			for (UINT i = 0; i < numUnknowns; i++) {
				IUnknown** ppUnknown = appUnknowns[i];
				if (*ppUnknown) {
					(*ppUnknown)->Release();
					(*ppUnknown) = nullptr;
				}
			}
		}
	} // namespace D3DHelper
} // namespace Skin
