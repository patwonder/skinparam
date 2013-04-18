/**
 * Interface for renderable objects
 */

#pragma once

#include "Camera.h"
#include "TString.h"

namespace Skin {
	class Renderable /* interface */ {
	protected:
		/**
		 * Common routines for fellow renderables
		 */
		// error reporting
		static void checkFailure(HRESULT hr, const Utils::TString& prompt);
		// shaders
		static HRESULT compileShader(const Utils::TString& strFileName, const char* szEntryPoint, const char* szShaderModel, ID3DBlob** ppBlobOut);
		static HRESULT loadVertexShaderAndLayout(ID3D11Device* pDevice,
			const Utils::TString& strFileName, const char* szEntryPoint,
			D3D11_INPUT_ELEMENT_DESC aLayoutDesc[], UINT numLayoutDesc,
			ID3D11VertexShader** ppVertexShader, ID3D11InputLayout** ppInputLayout);
		static HRESULT loadPixelShader(ID3D11Device* pDevice,
			const Utils::TString& strFileName, const char* szEntryPoint,
			ID3D11PixelShader** ppPixelShader);
		// buffers
		static HRESULT createBuffer(ID3D11Device* pDevice, UINT byteWidth, UINT bindFlags, void* pData, ID3D11Buffer** ppVertexBuffer); // for internal use
		template <class VertexType>
		static HRESULT createVertexBuffer(ID3D11Device* pDevice, VertexType aVertices[], UINT numVertices, ID3D11Buffer** ppVertexBuffer);
		template <class IndexType>
		static HRESULT createIndexBuffer(ID3D11Device* pDevice, IndexType aVertices[], UINT numIndices, ID3D11Buffer** ppIndexBuffer);
		// house keeping
		static void releaseCOMObjs(IUnknown** appUnknowns[], UINT numUnknowns);
	public:
		virtual void init(ID3D11Device* pDevice) = 0;
		virtual void render(ID3D11DeviceContext* pDeviceContext, const Camera& pCamera) = 0;
		virtual void cleanup() = 0;
		virtual ~Renderable() {}
	};

	// template implementation
	template <class VertexType>
	HRESULT Renderable::createVertexBuffer(ID3D11Device* pDevice, VertexType aVertices[], UINT numVertices, ID3D11Buffer** ppVertexBuffer) {
		return createBuffer(pDevice, sizeof(VertexType) * numVertices, D3D11_BIND_VERTEX_BUFFER, aVertices, ppVertexBuffer);
	}

	template <class IndexType>
	HRESULT Renderable::createIndexBuffer(ID3D11Device* pDevice, IndexType aIndices[], UINT numIndices, ID3D11Buffer** ppIndexBuffer) {
		return createBuffer(pDevice, sizeof(IndexType) * numIndices, D3D11_BIND_INDEX_BUFFER, aIndices, ppIndexBuffer);
	}
} // namespace Skin
