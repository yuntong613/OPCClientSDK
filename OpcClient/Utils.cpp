#include "stdafx.h"
#include "Utils.h"
#include <memory>
#include <iostream>
#include <codecvt>

using std::auto_ptr;

std::string CUtils::UnicodeToUTF8(const std::wstring & wstr)
{
	std::string ret;
	try {
		std::wstring_convert< std::codecvt_utf8<wchar_t> > wcv;
		ret = wcv.to_bytes(wstr);
	}
	catch (const std::exception & e) {
		std::cerr << e.what() << std::endl;
	}
	return ret;
}

std::wstring CUtils::UTF8ToUnicode(const std::string & str)
{
	std::wstring ret;
	try {
		std::wstring_convert< std::codecvt_utf8<wchar_t> > wcv;
		ret = wcv.from_bytes(str);
	}
	catch (const std::exception & e) {
		std::cerr << e.what() << std::endl;
	}
	return ret;
}

std::string CUtils::UnicodeToANSI(const std::wstring & wstr)
{
	std::string ret;
	std::mbstate_t state = {};
	const wchar_t *src = wstr.data();
	size_t len = std::wcsrtombs(nullptr, &src, 0, &state);
	if (static_cast<size_t>(-1) != len) {
		std::unique_ptr< char[] > buff(new char[len + 1]);
		len = std::wcsrtombs(buff.get(), &src, len, &state);
		if (static_cast<size_t>(-1) != len) {
			ret.assign(buff.get(), len);
		}
	}
	return ret;
}

std::wstring CUtils::ANSIToUnicode(const std::string & str)
{
	std::wstring ret;
	std::mbstate_t state = {};
	const char *src = str.data();
	size_t len = std::mbsrtowcs(nullptr, &src, 0, &state);
	if (static_cast<size_t>(-1) != len) {
		std::unique_ptr< wchar_t[] > buff(new wchar_t[len + 1]);
		len = std::mbsrtowcs(buff.get(), &src, len, &state);
		if (static_cast<size_t>(-1) != len) {
			ret.assign(buff.get(), len);
		}
	}
	return ret;
}

std::string CUtils::UTF8ToANSI(const std::string & str)
{
	return UnicodeToANSI(UTF8ToUnicode(str));
}

std::string CUtils::ANSIToUTF8(const std::string & str)
{
	return UnicodeToUTF8(ANSIToUnicode(str));
}
