/*
 * Shader encapsulation
 */

#pragma once

namespace RLSkin {
	// base class for shaders
	class Program;
	class Shader {
	private:
		Shader(const Shader&);
		Shader& operator=(const Shader&);
	protected:
		Shader(const Utils::TString& fileName, RLenum shaderType);

		RLshader m_shader;
	public:
		virtual ~Shader() = 0;
		friend class Program;
	};

	class FrameShader : public Shader {
	public:
		FrameShader(const Utils::TString& fileName)
			: Shader(fileName, RL_FRAME_SHADER) { }
	};

	class VertexShader : public Shader {
	public:
		VertexShader(const Utils::TString& fileName)
			: Shader(fileName, RL_VERTEX_SHADER) { }
	};

	class RayShader : public Shader {
	public:
		RayShader(const Utils::TString& fileName)
			: Shader(fileName, RL_RAY_SHADER) { }
	};
} // namespace RLSkin
