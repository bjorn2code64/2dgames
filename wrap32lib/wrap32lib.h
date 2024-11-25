#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <string>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#define _WINSOCKAPI_	// stop winsock.h being included 
#include <windows.h>

#define RETURN_IF_ERROR(x)		{ DWORD dwRet = (x); if (dwRet != ERROR_SUCCESS) return dwRet; }

#pragma warning( disable : 4100) 

extern void w32seed();
extern DWORD w32rand(DWORD lo, DWORD hi);
extern DWORD w32rand(DWORD hi);
float w32randf(float lo, float hi);
float w32randf(float hi);

extern void w32GetError(std::wstring& ret, DWORD dw);

template<class Interface>
inline void SafeRelease(Interface** ppInterfaceToRelease) {
	if (*ppInterfaceToRelease != NULL) {
		(*ppInterfaceToRelease)->Release();
		(*ppInterfaceToRelease) = NULL;
	}
}

int w32ReplaceStrings(std::string& source, LPCSTR szFrom, LPCSTR szTo);
