/*
 * Program encapsulation
 */

#include "stdafx.h"

#include "Program.h"
#include "RLHelper.h"

using namespace RLSkin;
using namespace Utils;
using namespace RLHelper;
using namespace std;

Program::Program(const vector<Shader*>& vpShaders) {
	m_program = rlCreateProgram();
	for (Shader* pShader : vpShaders) {
		pShader->attachToProgram(m_program);
	}
	rlLinkProgram(m_program);

	int linkStatus;
	rlGetProgramiv(m_program, RL_LINK_STATUS, &linkStatus);
	if(linkStatus == RL_FALSE) {
		const char* log;
		// Get the link errors and print them out.
		rlGetProgramString(m_program, RL_LINK_LOG, &log);
		TStringStream tss;
		tss << _T("Failed to link program: ") << log;
		checkFailure(E_FAIL, tss.str());
	}
}

Program::~Program() {
	rlDeleteProgram(m_program);
}

void Program::use() {
	rlUseProgram(m_program);
}

RLint Program::getAttributeLocation(const char* name) {
	return rlGetAttribLocation(m_program, name);
}

RLint Program::getUniformLocation(const char* name) {
	return rlGetUniformLocation(m_program, name);
}