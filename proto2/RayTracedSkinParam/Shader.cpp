/*
 * Shader encapsulation
 */

#include "stdafx.h"

#include "Shader.h"
#include "RLHelper.h"

using namespace RLSkin;
using namespace Utils;
using namespace RLHelper;

Shader::Shader(const TString& fileName, RLenum shaderType) {
	std::string source = readShaderSource(fileName);
	const char* lpszSource = source.c_str();
	RLsize len = source.length();
	m_shader = rlCreateShader(shaderType);
	rlShaderSource(m_shader, 1, &lpszSource, &len);
	rlCompileShader(m_shader);

	int compileStatus;
	rlGetShaderiv(m_shader, RL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == RL_FALSE) {
        const char* log;
        rlGetShaderString(m_shader, RL_COMPILE_LOG, &log);
		TStringStream tss;
		tss << _T("Failed to compile shader from file \"") << fileName << _T("\"\n") << log;
		checkFailure(E_FAIL, tss.str());
	}
}

Shader::~Shader() {
	rlDeleteShader(m_shader);
}

void Shader::attachToProgram(RLprogram program) {
	rlAttachShader(program, m_shader);
}

void Shader::detachFromProgram(RLprogram program) {
	rlDetachShader(program, m_shader);
}
