#pragma once

#include "wrap32lib.h"
#include <string>

class RegistryKey
{
public:
	RegistryKey() : m_hKey(NULL) {}
	RegistryKey(HKEY hKey, LPCWSTR subkey, bool bWrite = false) {
		if (RegOpenKeyEx(hKey, subkey, 0, (bWrite ? KEY_WRITE : 0) | KEY_READ | KEY_WOW64_64KEY, &m_hKey) != ERROR_SUCCESS) {
			if (bWrite) {	// create it?
				DWORD disposition;
				if (RegCreateKeyEx(hKey, subkey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ | KEY_WOW64_64KEY, NULL, &m_hKey, &disposition) != ERROR_SUCCESS)
					m_hKey = NULL;
			}
			else
				m_hKey = NULL;
		}
	}
	~RegistryKey() {
		Close();
	}

	void Close() {
		if (m_hKey)	RegCloseKey(m_hKey);
		m_hKey = NULL;
	}

	BOOL Open(LPCTSTR szKey, HKEY hKey = HKEY_LOCAL_MACHINE) {
		Close();
		return RegCreateKeyEx(hKey, szKey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &m_hKey, NULL) == ERROR_SUCCESS;
	}

	bool ReadString(LPCWSTR name, std::wstring& val) {
		DWORD l;
		DWORD type;
		if (RegQueryValueEx(m_hKey, name, NULL, &type, NULL, &l) != ERROR_SUCCESS)
			return false;

		if (type != REG_SZ)
			return false;

		LPWSTR data = new wchar_t[l];
		if (RegQueryValueEx(m_hKey, name, NULL, &type, (LPBYTE)data, &l) != ERROR_SUCCESS) {
			delete[] data;
			return false;
		}

		val = data;
		delete[] data;
		return true;
	}

	bool WriteString(LPCWSTR name, LPCWSTR val) {
		if (RegSetKeyValue(m_hKey, NULL, name, REG_SZ, val, (DWORD)(wcslen(val) + 1) * sizeof(wchar_t)) != ERROR_SUCCESS)
			return false;

		return true;
	}

protected:
	HKEY m_hKey;
};