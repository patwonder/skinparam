/*
 * program encapsulation
 */

#pragma once

#include "Shader.h"
#include "RLUnknown.h"

namespace RLSkin {
	// class that encapsulates an OpenRL Program reference
	class Program : public RLUnknown {
	private:
		RLprogram m_program;
		Program(const std::vector<Shader*>& vpShaders);
	protected:
		~Program() override;
	public:
		static void createInstance(const std::vector<Shader*>& vpShaders, Program** oppProgram) {
			*oppProgram = new Program(vpShaders);
		}

		void use();
		RLint getAttributeLocation(const char* name);
		RLint getUniformLocation(const char* name);
	};
} // namespace RLSkin
