/**
 * Test renderable object
 */
#include "stdafx.h"

#include "Triangle.h"
#include "FVector.h"

using namespace Skin;
using namespace Utils;

Triangle::Triangle()
	: m_pVertexShader(nullptr),
	  m_pPixelShader(nullptr),
	  m_pInputLayout(nullptr),
	  m_pVertexBuffer(nullptr)
{
}

Triangle::~Triangle() {
	cleanup();
}

void Triangle::init(ID3D11Device* pDevice) {
    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	UINT numElements = ARRAYSIZE(layout);

	// Load shaders
	checkFailure(loadVertexShaderAndLayout(pDevice, _T("Triangle.fx"), "VS", layout, numElements,
		&m_pVertexShader, &m_pInputLayout),
		_T("Failed to load vertex shader"));

	checkFailure(loadPixelShader(pDevice, _T("Triangle.fx"), "PS", &m_pPixelShader),
		_T("Failed to load pixel shader"));

    // Create vertex buffer
    FVector vertices[] = {
        FVector(0.0f, 0.5f, 0.5f),
        FVector(0.5f, -0.5f, 0.5f),
        FVector(-0.5f, -0.5f, 0.5f),
    };
	UINT numVertices = ARRAYSIZE(vertices);

	checkFailure(createVertexBuffer(pDevice, vertices, numVertices, &m_pVertexBuffer),
		_T("Failed to create vertex buffer"));
}

void Triangle::render(ID3D11DeviceContext* pDeviceContext, const Camera& pCamera) {
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	// Set vertex buffer
    UINT stride = sizeof(FVector);
    UINT offset = 0;
    pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	
	// Set primitive topology
    pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	pDeviceContext->VSSetShader(m_pVertexShader, NULL, 0);
	pDeviceContext->PSSetShader(m_pPixelShader, NULL, 0);
    pDeviceContext->Draw(3, 0);
}

void Triangle::cleanup() {
	IUnknown** ppUnknowns[] = {
		(IUnknown**)&m_pVertexShader,
		(IUnknown**)&m_pPixelShader,
		(IUnknown**)&m_pInputLayout,
		(IUnknown**)&m_pVertexBuffer
	};
	UINT num = ARRAYSIZE(ppUnknowns);

	releaseCOMObjs(ppUnknowns, num);
}
