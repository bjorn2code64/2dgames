#pragma once

#include "wrap32lib.h"

class ClipboardHandler
{
public:
	ClipboardHandler(HWND hWnd) : m_hWnd(hWnd) {}

	DWORD Copy(LPCWSTR sz) {
		size_t len = (wcslen(sz) + 1) * sizeof(wchar_t);
		if (!::OpenClipboard(m_hWnd)) {
			return GetLastError();
		}

		if (!::EmptyClipboard()) {
			::CloseClipboard();
			return GetLastError();
		}

		HANDLE hMem = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, len);
		if (!hMem) {
			::CloseClipboard();
			return GetLastError();
		}

		LPVOID pDest = ::GlobalLock(hMem);
		if (!pDest) {
			::GlobalUnlock(hMem);
			::CloseClipboard();
			return GetLastError();
		}

		memcpy(pDest, sz, len);
		::GlobalUnlock(hMem);
		::SetClipboardData(CF_UNICODETEXT, hMem);
		::CloseClipboard();
		return ERROR_SUCCESS;
	}

protected:
	HWND m_hWnd;
};