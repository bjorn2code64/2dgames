#pragma once

#include <wrap32lib.h>
#include <vector>
#include <sstream>

class VersionInfo
{
public:
	VersionInfo(LPCWSTR szPath = NULL);
	~VersionInfo(void);

	BOOL GetVersionNumbers(int* pVersions, int nLen);
	BOOL GetVersionNumbers(std::vector<int>& v);

	operator DWORD()	{	int v[4]; if (!GetVersionNumbers(v, 4))	return 0;	return ArrayToDWORD(v);	}

	static DWORD ArrayToDWORD(int* p)	{	return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];	}	// pack 4 ints into 4 bytes of an DWORD, first is heighest

	BOOL GetVersion(wchar_t* buff, size_t len) {
		int v[3];
		if (!GetVersionNumbers(v, 3))
			return FALSE;
		_snwprintf_s(buff, len, _TRUNCATE, L"v%d.%dr%d", v[0], v[1], v[2]);
		return TRUE;
	}

	std::wstring GetVersionAsString() {
		std::wostringstream woss;
		int v[4];
		if (GetVersionNumbers(v, 4)) {
			woss << v[0] << '.' << v[1] << '.' << v[2] << '.' << v[3];
		}
		return woss.str();
	}
protected:
	void* m_pVersion;
};
