/*
 * Helper functions for OpenRL
 */

#include "stdafx.h"

#include "RLHelper.h"

using namespace RLSkin;
using namespace RLHelper;
using namespace Utils;

void RLHelper::checkFailure(HRESULT hr, const TString& prompt) {
	if (FAILED(hr)) {
		TStringStream ss;
		ss << (prompt) << _T("\nError code: ") << hr;
		::MessageBox(AfxGetMainWnd()->m_hWnd, ss.str().c_str(), _T("Fatal Error"), MB_OK | MB_ICONERROR);
		::exit(EXIT_FAILURE);
	}
}

std::string RLHelper::readShaderSource(const TString& fileName) {
	CFile file;
	if (file.Open(fileName.c_str(), CFile::modeRead | CFile::shareDenyWrite)) {
		ULONGLONG len = file.GetLength();
		std::string buf;
		buf.resize(len);
		file.Read(&buf[0], (UINT)len);
		
		file.Close();
		return buf;
	} else {
		TStringStream tss;
		tss << _T("Failed to open shader file \"") << fileName << _T("\"");
		checkFailure(E_FAIL, tss.str());
	}
	return "";
}
