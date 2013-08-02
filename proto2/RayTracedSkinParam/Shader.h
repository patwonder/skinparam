/*
 * Shader encapsulation
 */

#pragma once

#include "RLUnknown.h"
#include "RLHelper.h"

namespace RLSkin {
	class Shader : public RLUnknown {
	private:
		RLshader m_shader;
		Shader(const Utils::TString& fileName, RLenum shaderType);
	protected:
		~Shader() override;
	public:
		static void createInstance(const Utils::TString& fileName, RLenum shaderType, Shader** oppShader) {
			*oppShader = new Shader(fileName, shaderType);
		}

		void attachToProgram(RLprogram program);
	};
} // namespace RLSkin
