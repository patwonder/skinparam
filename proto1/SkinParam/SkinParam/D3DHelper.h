/**
 * D3D helper functions
 */

#pragma once

#include "TString.h"
#include <wincodec.h>

namespace DirectX {
	enum DDS_ALPHA_MODE;
}

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
		HRESULT loadHullShader(ID3D11Device* pDevice,
			const Utils::TString& strFileName, const char* szEntryPoint,
			ID3D11HullShader** ppHullShader);
		HRESULT loadDomainShader(ID3D11Device* pDevice,
			const Utils::TString& strFileName, const char* szEntryPoint,
			ID3D11DomainShader** ppDomainShader);
		HRESULT loadGeometryShader(ID3D11Device* pDevice,
			const Utils::TString& strFileName, const char* szEntryPoint,
			ID3D11GeometryShader** ppGeometryShader);
		void clearShaderCache();
		// texture
		void getResourceSize(ID3D11ShaderResourceView* pSRV, UINT* pWidth, UINT* pHeight);
		void getResourceSize(ID3D11RenderTargetView* pRT, UINT* pWidth, UINT* pHeight);
		void getResourceSize(ID3D11Resource* pResource, UINT* pWidth, UINT* pHeight);
		HRESULT loadSRVFromWICFile(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const Utils::TString& strFileName,
			ID3D11ShaderResourceView** ppSRV);
		HRESULT loadSRVFromWICFileEx(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const Utils::TString& strFileName,
			size_t maxsize, D3D11_USAGE usage, unsigned int bindFlags, unsigned int cpuAccessFlags, unsigned int miscFlags, bool forceSRGB,
			ID3D11ShaderResourceView** ppSRV);
		HRESULT loadSRVFromMemory(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, void* pData, UINT width, UINT height, DXGI_FORMAT format,
			UINT rowPitch, ID3D11ShaderResourceView** ppSRV, bool autogen = true);
		bool isDDSFile(const Utils::TString& strFileName);
		HRESULT loadSRVFromDDSFile(ID3D11Device* pDevice, const Utils::TString& strFileName,
			ID3D11ShaderResourceView** ppSRV, DirectX::DDS_ALPHA_MODE* pAlphaMode = nullptr);
		HRESULT loadSRVFromDDSFileEx(ID3D11Device* pDevice, const Utils::TString& strFileName,
			size_t maxsize, D3D11_USAGE usage, unsigned int bindFlags, unsigned int cpuAccessFlags, unsigned int miscFlags, bool forceSRGB,
			ID3D11ShaderResourceView** ppSRV, DirectX::DDS_ALPHA_MODE* pAlphaMode = nullptr);
		HRESULT loadImageData(const Utils::TString& strFileName, void** ppData, UINT* pWidth, UINT* pHeight, WICPixelFormatGUID* pPixelFormatGUID);
		void freeImageData(void* pData);
		HRESULT createSamplerState(ID3D11Device* pDevice, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressU,
			D3D11_TEXTURE_ADDRESS_MODE addressV, D3D11_TEXTURE_ADDRESS_MODE addressW, ID3D11SamplerState** ppSamplerState);
		HRESULT createSamplerComparisonState(ID3D11Device* pDevice, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressU,
			D3D11_TEXTURE_ADDRESS_MODE addressV, D3D11_TEXTURE_ADDRESS_MODE addressW, D3D11_COMPARISON_FUNC comp, ID3D11SamplerState** ppSamplerState);
		HRESULT createSamplerStateEx(ID3D11Device* pDevice, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressU,
			D3D11_TEXTURE_ADDRESS_MODE addressV, D3D11_TEXTURE_ADDRESS_MODE addressW,
			XMFLOAT4 borderColor, UINT maxAnisotropy, ID3D11SamplerState** ppSamplerState);
		HRESULT createSamplerComparisonStateEx(ID3D11Device* pDevice, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressU,
			D3D11_TEXTURE_ADDRESS_MODE addressV, D3D11_TEXTURE_ADDRESS_MODE addressW, D3D11_COMPARISON_FUNC comp,
			XMFLOAT4 borderColor, UINT maxAnisotropy, ID3D11SamplerState** ppSamplerState);
		HRESULT createTexture2D(ID3D11Device* pDevice, UINT width, UINT height, DXGI_FORMAT format,
			ID3D11Texture2D** ppTexture2D, D3D11_BIND_FLAG bindFlags = D3D11_BIND_SHADER_RESOURCE);
		HRESULT createTexture2DEx(ID3D11Device* pDevice, UINT width, UINT height, DXGI_FORMAT format,
			bool multisampled, UINT multisampleCount, UINT multisampleQuality,
			ID3D11Texture2D** ppTexture2D, D3D11_BIND_FLAG bindFlags = D3D11_BIND_SHADER_RESOURCE);
		HRESULT createSRVFromTexture2D(ID3D11Device* pDevice, ID3D11Texture2D* pTexture2D, DXGI_FORMAT format,
			ID3D11ShaderResourceView** ppShaderResourceView);
		HRESULT createSRVFromTexture2DEx(ID3D11Device* pDevice, ID3D11Texture2D* pTexture2D, DXGI_FORMAT format, bool multisampled,
			ID3D11ShaderResourceView** ppShaderResourceView);
		HRESULT createShaderResourceView2D(ID3D11Device* pDevice, UINT width, UINT height, DXGI_FORMAT format,
			ID3D11ShaderResourceView** ppTextureResourceView, D3D11_BIND_FLAG bindFlags = D3D11_BIND_SHADER_RESOURCE);
		HRESULT createRTFromTexture2D(ID3D11Device* pDevice, ID3D11Texture2D* pTexture2D, DXGI_FORMAT format,
			ID3D11RenderTargetView** ppRenderTargetView);
		HRESULT createRTFromTexture2DEx(ID3D11Device* pDevice, ID3D11Texture2D* pTexture2D, DXGI_FORMAT format, bool multisampled,
			ID3D11RenderTargetView** ppRenderTargetView);
		HRESULT createIntermediateRenderTarget(ID3D11Device* pDevice, UINT width, UINT height, DXGI_FORMAT format,
			ID3D11Texture2D** ppTexture2D, ID3D11ShaderResourceView** ppShaderResourceView, ID3D11RenderTargetView** ppRenderTargetView);
		HRESULT createIntermediateRenderTargetEx(ID3D11Device* pDevice, UINT width, UINT height, DXGI_FORMAT format,
			bool multisampled, UINT multisampleCount, UINT multisampleQuality,
			ID3D11Texture2D** ppTexture2D, ID3D11ShaderResourceView** ppShaderResourceView, ID3D11RenderTargetView** ppRenderTargetView);
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
		// utility functions
		void extractPlanesFromFrustum(XMFLOAT4 aPlaneEquation[6], const XMMATRIX& matrix, bool bNormalize);
		void normalizePlane(XMFLOAT4& planeEquation);
		D3D11_VIEWPORT createViewport(UINT width, UINT height, UINT topLeftX = 0, UINT topLeftY = 0);

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
			return createBuffer(pDevice, sizeof(ConstantType), D3D11_BIND_CONSTANT_BUFFER, data, ppConstantBuffer);
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
