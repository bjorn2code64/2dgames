#pragma once

#include "wrap32lib.h"

#include <map>
#include <vector>

class Url
{
public:
	Url(LPCWSTR sz, BOOL bSkipHost) : m_bSSL(false), m_nPort(80) {
		Parse(sz, bSkipHost);
	}

	BOOL Parse(LPCWSTR sz, BOOL bSkipHost) {
		if (!bSkipHost) {
			if (!wcscmp(sz, L"http://")) {
				sz += 7;
				m_bSSL = false;
			}
			else if (!wcscmp(sz, L"https://")) {
				sz += 8;
				m_bSSL = true;
			}

			m_host.clear();
			while (isalnum(*sz) || (*sz == '.')) {
				m_host.push_back(*(sz++));
			}

			m_nPort = 80;
			if (*sz == ':') {
				m_nPort = _wtoi(++sz);
				while (isdigit(*sz))
					sz++;
			}
		}

		if (*sz != '/') {
			return FALSE;
		}
		sz++;

		m_path.clear();
		while (isalnum(*sz) || (*sz == '/')) {
			m_path.push_back(*(sz++));
		}

		if (*sz == '?') {
			sz++;
			for (;;) {
				std::wstring prop;
				while (isalnum(*sz))
					prop.push_back(*(sz++));

				if (!wcsncmp(sz, L"%5B%5D=", 7))
					sz += 7;
				else if (!wcsncmp(sz, L"[]=", 3))
					sz += 3;
				else if (*sz == '=') {
					sz++;
				}
				else
					return FALSE;

				std::wstring value;
				while (isalnum(*sz) || (*sz == '_'))
					value.push_back(*(sz++));
				auto it = m_values.find(prop);
				if (it == m_values.end()) {
					m_values[prop] = { value };
				}
				else {
					it->second.push_back(value);
				}
				if (*sz != '&')
					break;
				sz++;
			}
		}

		return TRUE;
	}

	bool m_bSSL;
	std::wstring m_host;
	int m_nPort;
	std::wstring m_path;
	std::map<std::wstring, std::vector<std::wstring>> m_values;
};
