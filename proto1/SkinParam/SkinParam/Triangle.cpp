/**
 * Test renderable object
 */
#include "stdafx.h"

#include "Triangle.h"
#include "FVector.h"

using namespace Skin;
using namespace Utils;
using namespace D3DHelper;

Triangle::Triangle()
	: m_pVertexBuffer(nullptr)
{
}

Triangle::~Triangle() {
	cleanup();
}

namespace Skin {
	struct SimpleVertex {
		XMFLOAT3 Pos;
		XMFLOAT4 Color;
	};
}

void Triangle::init(ID3D11Device* pDevice) {
    // Create vertex buffer
    SimpleVertex vertices[] =
    {
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT4( 0.0f, 1.0f, 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 0.0f, 1.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f ) },
    };
	UINT numVertices = ARRAYSIZE(vertices);

	checkFailure(createVertexBuffer(pDevice, vertices, numVertices, &m_pVertexBuffer),
		_T("Failed to create vertex buffer"));

	// Create index buffer
    WORD indices[] =
    {
        3,1,0,
        2,1,3,

        0,5,4,
        1,5,0,

        3,4,7,
        0,4,3,

        1,6,5,
        2,6,1,

        2,7,6,
        3,7,2,

        6,4,5,
        7,4,6,
    };
	UINT numIndices = ARRAYSIZE(indices);

	checkFailure(createIndexBuffer(pDevice, indices, numIndices, &m_pIndexBuffer),
		_T("Failed to create index buffer"));
}

void Triangle::render(ID3D11DeviceContext* pDeviceContext, const Camera& pCamera) {
	// Set vertex buffer
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	
	// Set index buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Set primitive topology
    pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// The draw call
	pDeviceContext->DrawIndexed(36, 0, 0);
}

void Triangle::cleanup() {
	IUnknown** ppUnknowns[] = {
		(IUnknown**)&m_pVertexBuffer,
		(IUnknown**)&m_pIndexBuffer
	};

	releaseCOMObjs(ppUnknowns);
}

XMMATRIX Triangle::getWorldMatrix() const {
	return XMMatrixIdentity();
}
