/**
 * TCHAR string classes
 */

#pragma once

#include <string>
#include <sstream>
#include <iomanip> // for setprecision, setiosflags, etc.

namespace Utils {
	typedef std::basic_string<TCHAR> TString;
	typedef std::basic_stringstream<TCHAR> TStringStream;
}
