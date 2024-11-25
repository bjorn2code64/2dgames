#pragma once

#include <wrap32lib.h>
#include "DateTime.h"

class File
{
public:
	File() : m_h(INVALID_HANDLE_VALUE), m_atEOF(false) {}
	File(LPCWSTR szPath, DWORD dwDesiredAccess, DWORD dwCreationDisposition, DWORD dwShare = 0) : m_atEOF(false) {
		Open(szPath, dwDesiredAccess, dwCreationDisposition, dwShare);
	}

	~File() {
		Close();
	}

	// dwDesiredAccess = GENERIC_READ | GENERIC_WRITE
	// dwCreationDisposition = CREATE_ALWAYS, CREATE_NEW, (unless it exists) OPEN_ALWAYS, OPEN_EXISTING, TRUNCATE_EXISTING
	// dwShare = FILE_SHARE_READ, FILE_SHARE_WRITE, FILE_SHARE_DELETE (Default no share at all)

	DWORD Open(LPCWSTR szPath, DWORD dwDesiredAccess, DWORD dwCreationDisposition, DWORD dwShare = 0) {
		m_h = ::CreateFile(szPath, dwDesiredAccess, dwShare, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
		m_atEOF = false;
		return m_h == INVALID_HANDLE_VALUE ? GetLastError() : ERROR_SUCCESS;
	}

	void Close() {
		if (m_h != INVALID_HANDLE_VALUE) {
			CloseHandle(m_h);
			m_h = INVALID_HANDLE_VALUE;
		}
	}

	DWORD Read(void* p, DWORD len, DWORD* pRead = NULL) {
		DWORD dwRead;
		if (!ReadFile(m_h, p, len, &dwRead, NULL))// && (dwRead == len))
			return GetLastError();

		if (pRead)
			*pRead = dwRead;

		m_atEOF = !dwRead && len;	// EOF returns TRUE but dwRead as 0
		return m_atEOF ? ERROR_HANDLE_EOF : ERROR_SUCCESS;
	}

	bool ReadLine(std::string& s) {
		s.clear();
		char ch;
		DWORD dwRead;
		while (ReadFile(m_h, &ch, 1, &dwRead, NULL) && (dwRead == 1)) {
			if (ch == '\r')	// expect a \n next char
				continue;
			if (ch == '\n')
				return true;
			s += ch;
		}
		return !s.empty();
	}

	DWORD Write(const void* p, DWORD len) {
		DWORD dwWritten;
		if (!WriteFile(m_h, p, len, &dwWritten, NULL)) //&& (dwWritten == len))
			return GetLastError();
		return ERROR_SUCCESS;
	}

	DWORD WriteBOM() {
		BYTE bom[] = { 0xff, 0xfe };
		return Write((const void*)bom, 2);
	}

	// Unicode file writing
	DWORD Write(LPCWSTR wsz) {
		return Write((const void*)wsz, DWORD(wcslen(wsz) * sizeof(wchar_t)));
	}

	// String utils for read/write of string in structures (e.g. with len first)
	bool WriteString(LPCWSTR wsz) {
		DWORD nLen = (DWORD)wcslen(wsz) + 1;	// cater for '\0'
		if (Write(&nLen, sizeof(nLen)) != ERROR_SUCCESS) return false;
		if (Write(wsz, sizeof(wchar_t) * nLen) != ERROR_SUCCESS) return false;
		return true;
	}

	bool ReadString(std::wstring& str) {
		DWORD nLen;
		if (Read(&nLen, sizeof(nLen)) != ERROR_SUCCESS)	return false;
		if (nLen > MAX_PATH + 1)	return false;
		wchar_t* p = new wchar_t[nLen];
		if (Read(p, sizeof(wchar_t) * nLen) != ERROR_SUCCESS) {
			delete[] p;
			return false;
		}
		if (p[nLen - 1] != '\0') {
			delete[]p;
			return false;
		}

		str = p;
		delete[]p;
		return true;
	}

	DWORD GetCreationDateTime(DateTime* pDTCreation) {
		FILETIME ftCreation;
		if (!::GetFileTime(
			m_h,
			&ftCreation,
			NULL,
			NULL
		)) {
			return GetLastError();
		}

		return pDTCreation->Set(ftCreation);
	}

	DWORD Rewind() {
		if (::SetFilePointer(m_h, 0, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
			return GetLastError();

		m_atEOF = false; // Assume not until read

		return ERROR_SUCCESS;
	}

	DWORD SetFilePointer(LONG bytes, DWORD dwMoveMethod = FILE_BEGIN) {
		if (::SetFilePointer(m_h, bytes, 0, dwMoveMethod) == INVALID_SET_FILE_POINTER) {
			DWORD dwError = GetLastError();
			if (dwError != NO_ERROR)
				return dwError;
		}

		return ERROR_SUCCESS;
	}

	DWORD SetFilePointer64(ULONGLONG bytes, DWORD dwMoveMethod = FILE_BEGIN) {
		ULONG ulLow = (ULONG)(bytes & 0xffffffff);
		ULONG ulHigh = (ULONG)(bytes >> 32);
		if (::SetFilePointer(m_h, *(PLONG)&ulLow, (PLONG)&ulHigh, dwMoveMethod) == INVALID_SET_FILE_POINTER) {
			DWORD dwError = GetLastError();
			if (dwError != NO_ERROR)
				return dwError;
		}

		return ERROR_SUCCESS;
	}

	operator HANDLE() { return m_h;  }

	bool AtEOF() { return m_atEOF;  }

	DWORD GetFileSize() {
		return (m_h == INVALID_HANDLE_VALUE) ? 0 : ::GetFileSize(m_h, NULL);
	}

	bool IsOpen() {
		return m_h != INVALID_HANDLE_VALUE;
	}

	BYTE* ReadAndAlloc(DWORD* pLen = NULL) {
		DWORD len = GetFileSize();
		if (!len) {
			return NULL;
		}

		BYTE* ret = new BYTE[len];
		if (!ret) {
			return NULL;
		}

		if (Read(ret, len, pLen) != ERROR_SUCCESS) {
			delete[] ret;
			return NULL;
		}

		return ret;
	}

protected:
	HANDLE m_h;
	bool m_atEOF;
};

