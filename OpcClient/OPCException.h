#pragma once
#include <string>
class OPCException
{
public:
	OPCException() { why = ""; rcode = S_OK; }
	OPCException(const std::string& what, HRESULT code = 0)
	{
		why = what;
		rcode = code;
	}

	~OPCException(){}

	std::string reasonString() { return why; }

	void reasonString(std::string what) { why = what; }

	HRESULT reasonCode() { return rcode; }

	void reasonCode(HRESULT code) { rcode = code; }

	std::string ErrorMessage()
	{
		void* pMsgBuf = NULL;

		::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, 
			rcode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&pMsgBuf, 0, NULL);
		char buff[2048] = { 0 };
		sprintf_s(buff, "%s, ErrorCode %08X, Cause %s", why.c_str(), rcode, (LPTSTR)pMsgBuf);
		// Free the buffer.
		LocalFree(pMsgBuf);

		std::string eText = buff;
		return eText;
	}
protected:
	std::string why;
	HRESULT rcode;
};

