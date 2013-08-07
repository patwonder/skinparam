/*
 * Helper functions for OpenRL
 */

#pragma once

#include "Buffer.h"

namespace RLSkin { namespace RLHelper {
	void checkFailure(HRESULT hr, const Utils::TString& prompt);
	std::string readShaderSource(const Utils::TString& fileName);
	template<class VertexType>
	void createVertexBuffer(RLenum usage, VertexType aVertices[], UINT numVertices, Buffer** oppBuffer);
	template<class UBType>
	void createUniformBuffer(const UBType& ub, Buffer** oppBuffer);
	template<class UBType>
	void updateUniformBuffer(Buffer* pBuffer, const UBType& ub);

	// Release functions
	void RLprogramRelease(RLprogram);
	void RLshaderRelease(RLshader);

	// template implementation
	template<class VertexType>
	void createVertexBuffer(RLenum usage, VertexType aVertices[], UINT numVertices, Buffer** oppBuffer) {
		UINT size = sizeof(VertexType) * numVertices;
		Buffer::createInstance(usage, oppBuffer, size, aVertices);
	}
	template<class UBType>
	void createUniformBuffer(const UBType& ub, Buffer** oppBuffer) {
		UINT size = sizeof(UBType);
		Buffer::createInstance(RL_DYNAMIC_DRAW, oppBuffer, size, &ub);
	}
	template<class UBType>
	void updateUniformBuffer(Buffer* pBuffer, const UBType& ub) {
		pBuffer->bind(RL_UNIFORM_BLOCK_BUFFER);
		rlBufferSubData(RL_UNIFORM_BLOCK_BUFFER, 0, sizeof(ub), &ub);
	}
} } // namespace RLSkin::RLHelper
