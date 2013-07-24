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

Program::Program(const vector<Shader*>& vShaders) {
	m_program = rlCreateProgram();
	for (Shader* pShader : vShaders) {
		rlAttachShader(m_program, pShader->m_shader);
		m_vAttached.push_back(pShader->m_shader);
	}
	rlLinkProgram(m_program);

	int linkStatus;
	rlGetProgramiv(m_program, RL_LINK_STATUS, &linkStatus);
	if(linkStatus == RL_FALSE) {
		const char* log;
		// Get the link errors and print them out.
		rlGetProgramString(m_program, RL_LINK_LOG, &log);
		TStringStream tss;
		tss << _T("Failed to link program: ") << TStringFromANSIString(log);
		checkFailure(E_FAIL, tss.str());
	}
}

Program::~Program() {
	for (RLshader shader : m_vAttached) {
		rlDetachShader(m_program, shader);
	}
	rlDeleteProgram(m_program);
}

void Program::use() {
	rlUseProgram(m_program);
}
