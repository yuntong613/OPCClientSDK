#include "stdafx.h"
#include "OPCException.h"

extern std::string g_ErrorText;
extern HRESULT g_ErrorCode;

OPCException::OPCException(const std::string& what, HRESULT code)
	:why(what),
	rcode(code)
{
	g_ErrorText = what;
	g_ErrorCode = code;
}


OPCException::~OPCException()
{
}

const std::string & OPCException::reasonString() const
{
	return why;
}

const HRESULT & OPCException::reasonCode() const
{
	return rcode;
}
