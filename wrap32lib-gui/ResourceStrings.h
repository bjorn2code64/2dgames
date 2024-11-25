#pragma once

#include "wrap32lib.h"
#include <map>

class ResourceString
{
public:
	ResourceString(int nID) : m_nID(nID), m_sz(NULL) { Load(); }
	~ResourceString() { if (m_sz)	delete[] m_sz; }

	const ResourceString& operator=(int nID) { m_nID = nID; return *this; }
	operator LPCWSTR () const;
	BOOL Get(LPCWSTR& sz);

protected:
	BOOL Load();

protected:
	LPWSTR m_sz;
	int m_nID;
};

class ResourceStrings
{
public:
	ResourceStrings() {}
	~ResourceStrings() {
		for (auto it : m_resStrings) {
			delete it.second;
		}
	}

	LPCWSTR operator[](UINT id) {
		return GetResString(id);
	}

	LPCWSTR GetResString(UINT id) {
		auto it = m_resStrings.find(id);
		if (it != m_resStrings.end()) {
			return (LPCWSTR)*(it->second);
		}
		ResourceString* p = new ResourceString(id);
		m_resStrings[id] = p;
		return (LPCWSTR)*p;
	}

protected:
	std::map<UINT, ResourceString*> m_resStrings;
};