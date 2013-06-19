/**
 * TCHAR string classes
 */

#include <Windows.h>
#include <tchar.h>
#include "TString.h"

using namespace Utils;

TString Utils::TStringFromANSIString(const std::string& str) {
	UINT cchSize = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, str.c_str(), -1, nullptr, 0);

	TString result;
	if (cchSize) {
		result.resize(cchSize - 1);
		cchSize = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, str.c_str(), -1, &result[0], cchSize);
	}

	if (cchSize == 0) {
		MessageBox(nullptr, _T("Failed to convert ansi string to tstring."), _T("Fatal Error"), MB_OK | MB_ICONERROR);
		exit(-1);
	}

	return result;
}

std::string Utils::ANSIStringFromTString(const TString& str) {
	UINT size = WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1,
		nullptr, 0, nullptr, nullptr);

	std::string result;
	if (size) {
		result.resize(size - 1);
		size = WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1,
			&result[0], size, nullptr, nullptr);
	}

	if (size == 0) {
		MessageBox(nullptr, _T("Failed to convert tstring to ansi string."), _T("Fatal Error"), MB_OK | MB_ICONERROR);
		exit(-1);
	}

	return result;
}

