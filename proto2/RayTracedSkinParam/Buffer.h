/*
 * Encapsulates an OpenRL buffer object
 */

#pragma once

#include "RLUnknown.h"

namespace RLSkin {
	class Buffer : public RLUnknown {
	private:
		RLbuffer m_buffer;

		Buffer(RLenum usage, UINT size = 0, const void* pData = nullptr);
	protected:
		~Buffer() override;
	public:
		static void createInstance(RLenum usage, Buffer** oppBuffer, UINT size = 0, const void* pData = nullptr) {
			*oppBuffer = new Buffer(usage, size, pData);
		}

		void bind(RLenum target = RL_ARRAY_BUFFER);
		void updateData(RLenum usage, UINT size = 0, const void* pData = nullptr);
	};
} // namespace RLSkin
