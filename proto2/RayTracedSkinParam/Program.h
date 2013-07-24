/*
 * program encapsulation
 */

#pragma once

#include "Shader.h"

namespace RLSkin {
	// class that encapsulates an OpenRL Program reference
	class Program {
	private:
		RLprogram m_program;
		std::vector<RLshader> m_vAttached;

		Program(const Program&);
		Program& operator=(const Program&);
	public:
		Program(const std::vector<Shader*>& vShaders);
		~Program();

		void use();
	};
} // namespace RLSkin
