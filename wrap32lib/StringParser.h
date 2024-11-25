#pragma once

#include "wrap32lib.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include "unicodemultibyte.h"

#define SP_SKIPSPACES		0x0001
#define SP_SKIPNEWLINES		0x0002
#define SP_SKIPTABS			0x0004
#define SP_SKIPWHITESPACE	(SP_SKIPSPACES | SP_SKIPNEWLINES | SP_SKIPTABS)

class StringParser
{
public:
	StringParser(LPCSTR sz, WORD flags = 0) : m_sz(sz) {
		if (flags & SP_SKIPSPACES)
			m_skips.push_back(' ');
		if (flags & SP_SKIPTABS)
			m_skips.push_back('\t');
		if (flags & SP_SKIPNEWLINES)
			m_skips.push_back('\n');

		Skip();
	}

	bool SplitInt(std::vector<int>& ret, char delim = ',') {
		int i;
		for (;;) {
			if (!GetInt(i))
				return false;
			ret.push_back(i);
			Skip();
			if (*m_sz == '\0')
				break;
			if (*m_sz != delim)
				return false;
			m_sz++;
		}
		return true;
	}

	bool GetInt(int& ret) {
		LPCSTR szStart = m_sz;
		ret = 0;
		while ((*m_sz >= '0') && (*m_sz <= '9')) {
			ret *= 10;
			ret += *m_sz - '0';
			m_sz++;
		}
		return m_sz != szStart;
	}

	bool SplitString(std::vector<std::string>& ret, char delim = ',', LPCSTR szQuotes = NULL) {
		std::string delims;
		delims.push_back(delim);
		delims.push_back(0);
		for (;;) {
			std::string str;
			if (!GetString(str, delims.c_str(), szQuotes))
				return false;
			ret.push_back(str);
			if (*m_sz == '\0')
				break;
			m_sz++;
		}
		return true;
	}

	void GetString(std::string& ret, char delim = '\0') {
		while (*m_sz && (*m_sz != delim)) {
			ret.push_back(*m_sz);
			m_sz++;
		}

		// Skip the delim (if it's not '\0')
		if (*m_sz && (*m_sz == delim))
			m_sz++;
	}

	bool GetString(std::string& ret, LPCSTR szDelim = ",", LPCSTR szQuotes = NULL) {
		char chQuote = 0;
		if (*m_sz && szQuotes && strchr(szQuotes, *m_sz)) {
			chQuote = *m_sz;
			m_sz++;
		}

		size_t delimLen = strlen(szDelim);
		while (*m_sz) {
			if (!chQuote) {	// no quotes
				if (!strncmp(szDelim, m_sz, delimLen))	// have we hit the end delim?
					break;
			}
			else {	// quotes
				if (*m_sz == chQuote)	// have we hit the end quote?
					break;
			}

			ret.push_back(*m_sz);
			m_sz++;
		}

		if (chQuote) {
			if (*m_sz != chQuote)
				return false;

			m_sz++;
		}

		return true;
	}

	bool ExpectString(LPCSTR sz, LPCSTR szDelim = ",", LPCSTR szQuotes = NULL) {
		std::string s;
		if (!GetString(s, szDelim, szQuotes))
			return false;
		return s == sz;
	}

	void GetString(std::wstring& ret, char delim = '\0') {
		std::string s;
		GetString(s, delim);
		ret = UnicodeMultibyte(s);
	}

	bool IsEmpty() { return *m_sz != 0; }

	int Skip() {
		int ret = 0;
		while (std::find(m_skips.begin(), m_skips.end(), *m_sz) != m_skips.end())
			m_sz++, ret++;
		return ret;
	}

	size_t Skip(char ch) {
		LPCSTR sz = m_sz;
		while (*m_sz == ch)
			m_sz++;
		return m_sz - sz;
	}

	void GetBool(bool& ret, char delim = '\0') {
		std::string s;
		GetString(s, delim);

		std::transform(s.begin(), s.end(), s.begin(),
			[](char c) { return (char)std::tolower((int)c); });

		ret = (!s.compare("y") || !s.compare("yes") || !s.compare("1") || !s.compare("true"));
	}

protected:
	LPCSTR m_sz;
	std::vector<char> m_skips;
};

class WStringParser
{
public:
	WStringParser(LPCWSTR sz, WORD flags = 0) : m_sz(sz) {
		if (flags & SP_SKIPSPACES)
			m_skips.push_back(L' ');
		if (flags & SP_SKIPTABS)
			m_skips.push_back('\t');
		if (flags & SP_SKIPNEWLINES)
			m_skips.push_back('\n');

		Skip();
	}

	bool SplitInt(std::vector<int>& ret, char delim = ',') {
		int i;
		for (;;) {
			if (!GetInt(i))
				return false;
			ret.push_back(i);
			Skip();
			if (*m_sz == '\0')
				break;
			if (*m_sz != delim)
				return false;
			m_sz++;
		}
		return true;
	}

	bool SplitUInt(std::vector<unsigned int>& ret, wchar_t delim = ',') {
		unsigned int i;
		for (;;) {
			if (!GetUInt(i))
				return false;
			ret.push_back(i);
			if (*m_sz == '\0')
				break;
			if (*m_sz != delim)
				return false;
			m_sz++;
		}
		return true;
	}

	bool GetUInt(unsigned int& ret) {
		LPCWSTR sz = m_sz;
		ret = 0;
		bool digitFound = false;
		while ((*sz >= '0') && (*sz <= '9')) {
			ret *= 10;
			ret += *sz - '0';
			sz++;
			digitFound = true;
		}

		if (!digitFound)
			return false;

		m_sz = sz;
		return true;
	}

	bool GetChar(wchar_t& ch) {
		if (*m_sz == '\0') {
			return false;
		}
		ch = *(m_sz++);
		return true;
	}

	bool GetInt(int& ret) {
		LPCWSTR sz = m_sz;
		bool bMinus = false;
		ret = 0;
		if (*sz == '-') {
			bMinus = true;
			sz++;
		}
		else if (*sz == '+')
			sz++;

		bool digitFound = false;
		while ((*sz >= '0') && (*sz <= '9')) {
			ret *= 10;
			ret += *sz - '0';
			sz++;
			digitFound = true;
		}

		if (!digitFound)	// a double
			return false;

		if (bMinus)
			ret = -ret;

		m_sz = sz;
		return true;
	}

	bool GetHexInt(int& ret) {
		bool bMinus = false;
		ret = 0;
		if (*m_sz == '-') {
			bMinus = true;
			m_sz++;
		}
		else if (*m_sz == '+')
			m_sz++;

		LPCWSTR szStart = m_sz;
		while (((*m_sz >= '0') && (*m_sz <= '9')) ||
				((*m_sz >= 'A') && (*m_sz <= 'F')) ||
				((*m_sz >= 'a') && (*m_sz <= 'f'))
			)
		{
			if (ret & 0x70000000)	// overflow!
				return false;
			ret *= 16;
			if ((*m_sz >= '0') && (*m_sz <= '9'))
				ret += *m_sz - '0';
			else if ((*m_sz >= 'a') && (*m_sz <= 'f'))
				ret += 10 + *m_sz - 'a';
			else
				ret += 10 + *m_sz - 'A';
			m_sz++;
		}

		if (bMinus)
			ret = -ret;

		return m_sz != szStart;
	}

	bool GetDouble(double& ret, bool& pointFound) {
		pointFound = false;
		bool bMinus = false;
		bool digitFound = false;
		ret = 0;
		if (*m_sz == '-') {
			bMinus = true;
			m_sz++;
		}
		else if (*m_sz == '+')
			m_sz++;

		while ((*m_sz >= '0') && (*m_sz <= '9')) {
			digitFound = true;
			ret *= 10;
			ret += *m_sz - '0';
			m_sz++;
		}

		if (*m_sz == '.') {
			pointFound = true;
			m_sz++;
			int dp = 0;
			int div = 1;
			while ((*m_sz >= '0') && (*m_sz <= '9')) {
				digitFound = true;
				dp += *m_sz - '0';
				dp *= 10;
				div *= 10;
				m_sz++;
			}

			if (dp)
				ret += (double)dp / (double)div;
		}

		if (bMinus)
			ret = -ret;

		return digitFound;
	}

	bool SplitString(std::vector<std::wstring>& ret, wchar_t delim = ',', LPCWSTR wszQuotes = NULL) {
		std::wstring delims;
		delims.push_back(delim);
		delims.push_back(0);
		for (;;) {
			std::wstring str;
			if (!GetString(str, delims.c_str(), wszQuotes))
				return false;
			ret.push_back(str);
			if (*m_sz == '\0')
				break;
			m_sz++;
		}
		return true;
	}

	bool GetString(std::wstring& ret, LPCWSTR wszDelims = L",", LPCWSTR wszQuotes = NULL) {
		wchar_t chQuote = 0;
		if (*m_sz && wszQuotes && wcschr(wszQuotes, *m_sz)) {
			chQuote = *m_sz;
			m_sz++;
		}

		while (*m_sz && (chQuote || !wcschr(wszDelims, *m_sz)) && (!chQuote || (*m_sz != chQuote))) {
			ret.push_back(*m_sz);
			m_sz++;
		}

		if (chQuote) {
			if (*m_sz != chQuote)
				return false;

			m_sz++;
		}

		return true;
	}

	int Skip() {
		int ret = 0;
		while (std::find(m_skips.begin(), m_skips.end(), *m_sz) != m_skips.end())
			m_sz++, ret++;
		return ret;
	}

	void SkipSpaces() {
		while (*m_sz == ' ')
			m_sz++;
	}

	int SkipWhiteSpace() {
		int newlines = 0;
		while ((*m_sz == ' ') || (*m_sz == '\r') || (*m_sz == '\n') || (*m_sz == '\t')) {
			if (*m_sz == '\n')
				newlines++;
			m_sz++;
		}
		return newlines;
	}

	BOOL IsEmpty() { return *m_sz == 0; }

	BOOL ExpectChar(wchar_t w) {
		if (*m_sz != w)
			return FALSE;
		m_sz++;
		return TRUE;
	}

	BOOL ExpectString(const wchar_t* w) {
		size_t len = wcslen(w);
		if (wcsncmp(m_sz, w, len))
			return FALSE;
		m_sz += len;
		return TRUE;
	}

	BOOL PeekChar(wchar_t w) {
		return *m_sz == w;
	}

	size_t GetLength() { return wcslen(m_sz); }

protected:
	LPCWSTR m_sz;
	std::vector<wchar_t> m_skips;
};