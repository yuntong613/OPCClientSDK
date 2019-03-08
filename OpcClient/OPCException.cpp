#include "stdafx.h"
#include "OPCException.h"


OPCException::OPCException(const std::string& what, HRESULT code)
	:why(what),
	rcode(code)
{
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
