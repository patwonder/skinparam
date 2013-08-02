#include "stdafx.h"

#include "Buffer.h"

using namespace RLSkin;

Buffer::Buffer(RLenum usage, UINT size, const void* pData) {
	rlGenBuffers(1, &m_buffer);
	updateData(usage, size, pData);
}

Buffer::~Buffer() {
	rlDeleteBuffers(1, &m_buffer);
}

void Buffer::updateData(RLenum usage, UINT size, const void* pData) {
	rlBindBuffer(RL_ARRAY_BUFFER, m_buffer);
	rlBufferData(RL_ARRAY_BUFFER, size, pData, usage);
}

void Buffer::bind(RLenum target) {
	rlBindBuffer(target, m_buffer);
}
