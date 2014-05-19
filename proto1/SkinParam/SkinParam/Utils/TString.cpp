
/*
    Copyright(c) 2013-2014 Yifan Wu.

    This file is part of SkinParam.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

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

