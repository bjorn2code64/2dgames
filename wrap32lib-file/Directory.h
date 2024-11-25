#pragma once

#include "wrap32lib.h"

#include <string>
#include <vector>

class Directory
{
public:
	Directory(LPCWSTR path) : m_path(path) {
	}

	DWORD GetContents(std::vector<std::wstring>& contents, bool folders, bool files, LPCWSTR mask = L"*.*") {
		WIN32_FIND_DATA ffd;
		std::wstring path = m_path + L"\\" + mask;
		HANDLE hFind = ::FindFirstFile(path.c_str(), &ffd);
		if (hFind == INVALID_HANDLE_VALUE)
			return GetLastError();

		do {
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (folders) {
					if (wcscmp(ffd.cFileName, L".") && wcscmp(ffd.cFileName, L".."))
						contents.push_back(ffd.cFileName);
				}
			}
			else {
				if (files) {
					contents.push_back(ffd.cFileName);
				}
			}

		} while (FindNextFile(hFind, &ffd) != 0);

		return ERROR_SUCCESS;
	}

	bool Exists() {
		DWORD dwAttrib = GetFileAttributes(m_path.c_str());
		return ((dwAttrib != INVALID_FILE_ATTRIBUTES) && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
	}

	void AddSubFolder(LPCWSTR subFolder) {
		m_path += L"\\";
		m_path += subFolder;
	}

	bool RemoveSubFolder() {
		auto it = m_path.rfind(L"\\");
		if (it == std::wstring::npos)
			return false;

		m_path = m_path.substr(0, it);
		return true;
	}

	bool Create(bool checkFullPath = false) {
		if (checkFullPath) {
			LPCWSTR szStart = m_path.c_str();
			wchar_t buff[MAX_PATH];
			buff[0] = '\0';
			while (szStart) {
				LPCWSTR szEnd = wcschr(szStart, '/');
				if (!szEnd)
					szEnd = wcschr(szStart, '\\');

				if (szEnd)
					szEnd++;

				size_t lenToCopy = szEnd ? szEnd - szStart : wcslen(szStart);
				size_t len = wcslen(buff);
				wcsncpy_s(buff + len, MAX_PATH - len, szStart, lenToCopy);
				buff[len + lenToCopy] = '\0';
				Directory dirTemp(buff);
				if (!dirTemp.Exists()) {
					if (!CreateDirectory(buff, NULL))
						return false;
				}
				szStart = szEnd;
			}
			return true;
		}
		return CreateDirectory(m_path.c_str(), NULL) != FALSE;
	}

	operator LPCWSTR() { return m_path.c_str();  }

protected:
	std::wstring m_path;
};