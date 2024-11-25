#pragma once

#include "wrap32lib.h"
#include "StringParser.h"
#include "unicodemultibyte.h"

#include <vector>
#include <string>

class CmdLine
{
public:
	CmdLine(LPCWSTR lpCmdLine) {
		WStringParser wsp(lpCmdLine, SP_SKIPSPACES);
		if (!wsp.IsEmpty()) {
			wsp.SplitString(m_args, ' ', L"\"");
		}
	}

	size_t ArgCount() const { return m_args.size();	}
	LPCWSTR Arg(int i) const { return m_args[i].c_str(); }

	size_t GetArgIndex(const wchar_t* prefix) const {
		for (size_t i = 0; i < m_args.size(); i++) {
			if (m_args[i].compare(prefix) == 0) {
				return i + 1;
			}
		}
		return 0;
	}

	bool ArgExists(const wchar_t* prefix) const {
		return GetArgIndex(prefix) != 0;
	}

	bool GetArgWString(const wchar_t* prefix, std::wstring& ret) const {
		size_t index = GetArgIndex(prefix);
		if ((index == 0) || (index >= m_args.size()))
			return false;

		ret = m_args[index];
		return true;
	}

	bool GetArgString(const wchar_t* prefix, std::string& ret) const {
		std::wstring wstr;
		if (!GetArgWString(prefix, wstr))
			return false;

		ret = UnicodeMultibyte(wstr);
		return true;
	}

	bool GetArgWORD(const wchar_t* prefix, WORD& ret) const {
		std::string str;
		if (!GetArgString(prefix, str))
			return false;

		ret = (WORD)std::stoi(str);
		return true;
	}

	bool GetArgDWORD(const wchar_t* prefix, DWORD& ret) const {
		std::string str;
		if (!GetArgString(prefix, str))
			return false;

		try {
			ret = (DWORD)std::stoi(str);
		}
		catch (...) {
			return false;
		}
		return true;
	}

	bool GetArgBool(const wchar_t* prefix, bool& ret) const {
		std::string str;
		if (!GetArgString(prefix, str))
			return false;

		StringParser sp(str.c_str());
		sp.GetBool(ret);
		return true;
	}

protected:
	std::vector<std::wstring> m_args;
};
