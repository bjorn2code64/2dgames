#pragma once

#include "wrap32lib.h"

class UnicodeMultibyte {
public:
	UnicodeMultibyte(LPCSTR sz) : m_sz(NULL), m_wsz(NULL)				{	Init(sz);  }
	UnicodeMultibyte(LPCWSTR wsz) : m_sz(NULL), m_wsz(NULL)				{	Init(wsz); }
	UnicodeMultibyte(const std::string& s) : m_sz(NULL), m_wsz(NULL)	{	Init(s.c_str());		}
	UnicodeMultibyte(const std::wstring& w) : m_sz(NULL), m_wsz(NULL)	{	Init(w.c_str());	}
	UnicodeMultibyte(const UnicodeMultibyte& rhs)						{	Init(rhs.m_sz);	Init(rhs.m_wsz);	}

	~UnicodeMultibyte() {
		if (m_sz)
			delete[] m_sz;
		else if (m_wsz)
			delete[] m_wsz;
	}

	const UnicodeMultibyte& operator=(const UnicodeMultibyte& rhs) {
		Init(rhs.m_sz);	Init(rhs.m_wsz);
		return *this;
	}

	const UnicodeMultibyte& operator=(const std::wstring& w) {
		Init(w.c_str());
		return *this;
	}

	const UnicodeMultibyte& operator=(LPCWSTR wsz) {
		Init(wsz);
		return *this;
	}

	const UnicodeMultibyte& operator=(const std::string& s) {
		Init(s.c_str());
		return *this;
	}

	const UnicodeMultibyte& operator=(LPCSTR sz) {
		Init(sz);
		return *this;
	}

	operator LPCSTR() {
		if (!m_sz) {
			DWORD size = WideCharToMultiByte(CP_OEMCP, NULL, m_wsz, -1, NULL, 0, NULL, FALSE);
			m_sz = new char[size];
			WideCharToMultiByte(CP_OEMCP, NULL, m_wsz, -1, (LPSTR)m_sz, size, NULL, FALSE);
		}
		return m_sz;
	}

	operator LPCWSTR() {
		if (!m_wsz) {
			DWORD size = MultiByteToWideChar(CP_ACP, 0, m_sz, -1, NULL, 0);
			m_wsz = new wchar_t[size];
			MultiByteToWideChar(CP_ACP, 0, m_sz, -1, (LPWSTR)m_wsz, size);
		}
		return m_wsz;
	}

protected:
	void Init(LPCWSTR wsz) {
		if (m_wsz)
			delete[] m_wsz;
		if (m_sz) {
			delete[] m_sz;
			m_sz = NULL;
		}

		if (wsz) {
			size_t len = wcslen(wsz) + 1;
			m_wsz = new wchar_t[len];
			wcscpy_s((wchar_t*)m_wsz, len, wsz);
		}
		else {
			m_wsz = NULL;
		}
	}

	void Init(LPCSTR sz) {
		if (m_sz)
			delete[] m_sz;
		if (m_wsz) {
			delete[] m_wsz;
			m_wsz = NULL;
		}

		if (sz) {
			size_t len = strlen(sz) + 1;
			m_sz = new char[len];
			strcpy_s((char*)m_sz, len, sz);
		}
		else {
			m_sz = NULL;
		}

	}

protected:
	LPCSTR m_sz;
	LPCWSTR m_wsz;
};

