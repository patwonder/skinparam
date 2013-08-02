/*
 * Encapsulates an OpenRL primitive object
 */

#pragma once

#include "RLUnknown.h"

namespace RLSkin {
	class Program;
	class Primitive : public RLUnknown {
	private:
		RLprimitive m_primitive;

		Primitive();
	protected:
		~Primitive() override;
	public:
		static void createInstance(Primitive** oppPrimitive) {
			*oppPrimitive = new Primitive();
		}

		void bind();
		void useProgram(Program* pProgram);
	};
} // namespace RLSkin
