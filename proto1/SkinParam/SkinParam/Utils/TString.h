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
	typedef std::basic_ostream<TCHAR> TOstream;

	TString TStringFromANSIString(const std::string& str);
	std::string ANSIStringFromTString(const TString& str);
} // namespace Utils
