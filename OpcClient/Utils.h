#pragma once
#pragma warning (disable : 4996)

#include <strstream>

using namespace std;

#define CONVERT_UTF8(x)	  CUtils::ANSIToUTF8(x)
#define CONVERT_ACP(x)  CUtils::UTF8ToANSI(x)

class CUtils
{
public:
	static std::string UnicodeToUTF8(const std::wstring & wstr);
	static std::wstring UTF8ToUnicode(const std::string & str);
	static std::string UnicodeToANSI(const std::wstring & wstr);
	static std::wstring ANSIToUnicode(const std::string & str);
	static std::string UTF8ToANSI(const std::string & str);
	static std::string ANSIToUTF8(const std::string & str);
};
