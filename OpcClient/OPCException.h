#pragma once
#include <atlexcept.h>
#include <string>
class OPCException : public ATL::CAtlException
{
public:
	OPCException(const std::string& what, HRESULT code = 0);
	~OPCException();
	const std::string & reasonString() const;
	const HRESULT & reasonCode() const;
protected:
	std::string why;
	HRESULT rcode;
};

