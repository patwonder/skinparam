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
	
}

namespace Skin {
	struct SimpleVertex {
		XMFLOAT3 pos;
		XMFLOAT4 color;
		XMFLOAT3 normal;
		XMFLOAT2 texCoord;
	};
}

void Triangle::init(ID3D11Device* pDevice, IRenderer* pRenderer) {
    // Create vertex buffer
    SimpleVertex vertices[] = {
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT4( 0.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT4( 0.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT4( 0.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 0.0f, 1.0f, 1.0f ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 0.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 0.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        //{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        //{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        //{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        //{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        //{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        //{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        //{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        //{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        //{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        //{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        //{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        //{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        //{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        //{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        //{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        //{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        //{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        //{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        //{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        //{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        //{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        //{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        //{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        //{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
    };
	UINT numVertices = ARRAYSIZE(vertices);

	checkFailure(createVertexBuffer(pDevice, vertices, numVertices, &m_pVertexBuffer),
		_T("Failed to create vertex buffer"));

	// Create index buffer
    WORD indices[] =
    {
        10,4,1,
        7,4,10,

        2,17,14,
        5,17,2,

        9,12,21,
        0,12,9,

        3,18,15,
        6,18,3,

        8,23,20,
        11,23,8,

        19,13,16,
        22,13,19,
    };
	UINT numIndices = ARRAYSIZE(indices);

	checkFailure(createIndexBuffer(pDevice, indices, numIndices, &m_pIndexBuffer),
		_T("Failed to create index buffer"));
}

void Triangle::render(ID3D11DeviceContext* pDeviceContext, IRenderer* pRenderer, const Camera& pCamera) {
	// Set vertex buffer
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	
	// Set index buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Setup material and world matrix
	pRenderer->setWorldMatrix(XMMatrixRotationAxis(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), 2 * (float)Math::PI * ((GetTickCount() % 30000) / 30000.0f)));
	pRenderer->setMaterial(Material(Color::White, Color::White, Color::White, Color::Black, 5.0f));

	// The draw call
	pDeviceContext->DrawIndexed(36, 0, 0);
}

void Triangle::cleanup(IRenderer* pRenderer) {
	IUnknown** ppUnknowns[] = {
		(IUnknown**)&m_pVertexBuffer,
		(IUnknown**)&m_pIndexBuffer
	};

	releaseCOMObjs(ppUnknowns);
}
