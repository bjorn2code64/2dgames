#include "json.h"

namespace json {

	jsonArray* jsonObject::AddArrayProperty(LPCWSTR szName) {
		jsonArray* ja = new jsonArray;
		auto it = m_properties.find(szName);
		if (it != m_properties.end()) {
			delete it->second;
			it->second = ja;
		}
		else
			m_properties[szName] = ja;
		return ja;
	}
	jsonObject* jsonObject::AddObjectProperty(LPCWSTR szName) {
		jsonObject* jo = new jsonObject;
		auto it = m_properties.find(szName);
		if (it != m_properties.end()) {
			delete it->second;
			it->second = jo;
		}
		else
			m_properties[szName] = jo;
		return jo;
	}

	// Legacy Start
	void jsonObject::SetProperty(LPCWSTR szName, const jsonArray& array) {
		auto it = m_properties.find(szName);
		if (it != m_properties.end()) {
			delete it->second;
			it->second = new jsonArray(array);
		}
		else
			m_properties[szName] = new jsonArray(array);
	}

	void jsonObject::SetProperty(LPCWSTR szName, const jsonObject& object) {
		auto it = m_properties.find(szName);
		if (it != m_properties.end()) {
			delete it->second;
			it->second = new jsonObject(object);
		}
		else
			m_properties[szName] = new jsonObject(object);
	}
	// Legacy End

	bool jsonObject::Parse(WStringParser& wsp, int& line, std::wstring& err) {
		line += wsp.SkipWhiteSpace();
		if (!wsp.ExpectChar('{')) {
			err = L"Expected {";
			return false;
		}

		// object is series of "string" : objs
		line += wsp.SkipWhiteSpace();
		if (wsp.ExpectChar('}'))
			return true;

		for (;;) {
			line += wsp.SkipWhiteSpace();
			std::wstring name;
			if (!wsp.GetString(name, L":", L"\"")) {
				err = L"Expected property name";
				return false;
			}

			line += wsp.SkipWhiteSpace();
			if (!wsp.ExpectChar(':')) {
				err = L"Expected :";
				return false;
			}

			line += wsp.SkipWhiteSpace();

			if (wsp.PeekChar('{')) { // object
				jsonObject* pjo = new jsonObject;
				if (!pjo->Parse(wsp, line, err)) {
					delete pjo;
					return false;
				}
				m_properties[name] = pjo;
			}
			else if (wsp.PeekChar('[')) { // array
				jsonArray* pja = new jsonArray;
				if (!pja->Parse(wsp, line, err)) {
					delete pja;
					return false;
				}
				m_properties[name] = pja;
			}
			else {	// string?
				if (wsp.PeekChar('\"')) {
					std::wstring value;
					if (!wsp.GetString(value, L",}", L"\"")) {
						err = L"Expected , or }";
						return false;
					}

					m_properties[name] = new jsonString(value.c_str());
				}
				else {
					double value;
					bool pointFound;
					if (wsp.GetDouble(value, pointFound)) {
						if (pointFound)
							m_properties[name] = new jsonDouble(value);
						else
							m_properties[name] = new jsonInt((int)value);
					}
					else {
						std::wstring temp;
						if (!wsp.GetString(temp, L" \t\n\r,}")) {
							err = L"Expected value";
							return false;
						}
						
						if (!_wcsicmp(temp.c_str(), L"true")) {
							m_properties[name] = new jsonBool(true);
						}
						else if (!_wcsicmp(temp.c_str(), L"false")) {
							m_properties[name] = new jsonBool(false);
						}
						else if (!_wcsicmp(temp.c_str(), L"null")) {
							m_properties[name] = new jsonNull();
						}
						else {
							err = L"Expected true / false";
							return false;
						}
					}
				}
			}
			line += wsp.SkipWhiteSpace();

			if (wsp.ExpectChar('}'))
				break;

			line += wsp.SkipWhiteSpace();
			if (!wsp.ExpectChar(',')) {
				err = L"Expected ,";
				return false;
			}
		}
		return true;
	}

	void jsonObject::Span(std::function<void(jsonBase*)> f) {
		for (auto p : m_properties) {
			f(p.second);

			switch (p.second->GetType()) {
			case jsonType::typeArray:
				((jsonArray*)p.second)->Span(f);
				break;

			case jsonType::typeObject:
				((jsonObject*)p.second)->Span(f);
				break;
			}
		}
	}

	json::jsonBase* jsonObject::GetElementByPath(LPCWSTR path) const {
		LPCWSTR start = path;
		while (*path && (*path != '.') && (*path != '['))
			path++;
		std::wstring name = start;
		name = name.substr(0, path - start);
		jsonBase* jb = GetProperty(name.c_str());
		if (!jb)
			return NULL;

		if (!*path)
			return jb;

		if (*path == '.')
			path++;

		switch (jb->GetType()) {
		case typeObject:	return ((json::jsonObject*)jb)->GetElementByPath(path);
		case typeArray:		return ((json::jsonArray*)jb)->GetElementByPath(path);
		default:
			if (!*path)		return jb;
			break;
		}
		return NULL;
	}

	bool jsonArray::Parse(WStringParser& wsp, int& line, std::wstring& err) {
		line += wsp.SkipWhiteSpace();
		if (!wsp.ExpectChar('[')) {
			err = L"Expected [";
			return false;
		}

		// contents are a series of comma separated json objects
		line += wsp.SkipWhiteSpace();
		if (wsp.ExpectChar(']'))
			return true;

		for (;;) {
			line += wsp.SkipWhiteSpace();

			if (wsp.PeekChar('{')) { // object
				jsonObject* pjo = new jsonObject;
				if (!pjo->Parse(wsp, line, err)) {
					delete pjo;
					return false;
				}
				m_array.push_back(pjo);
			}
			else if (wsp.PeekChar('[')) { // array
				jsonArray* pja = new jsonArray;
				if (!pja->Parse(wsp, line, err)) {
					delete pja;
					return false;
				}
				m_array.push_back(pja);
			}
			else {	// string?
				if (wsp.PeekChar('\"')) {
					std::wstring value;
					if (!wsp.GetString(value, L",}", L"\"")) {
						err = L"Expected value";
						return false;
					}
					m_array.push_back(new jsonString(value.c_str()));
				}
				else {
					double value;
					bool pointFound;
					if (wsp.GetDouble(value, pointFound)) {
						if (pointFound)
							m_array.push_back(new jsonDouble(value));
						else
							m_array.push_back(new jsonInt((int)value));
					}
					else {
						err = L"Expected number";
						return false;
					}
				}
			}
			line += wsp.SkipWhiteSpace();

			if (wsp.ExpectChar(']'))
				break;

			line += wsp.SkipWhiteSpace();
			if (!wsp.ExpectChar(',')) {
				err = L"Expected ,";
				return false;
			}
		}
		return true;
	}

	void jsonArray::Span(std::function<void(jsonBase*)> f) {
		for (auto p : m_array) {
			f(p);

			switch (p->GetType()) {
			case jsonType::typeArray:
				((jsonArray*)p)->Span(f);
				break;

			case jsonType::typeObject:
				((jsonObject*)p)->Span(f);
				break;
			}
		}
	}

	json::jsonBase* jsonArray::GetElementByPath(LPCWSTR path) const {
		if (*path != '[')
			return NULL;
		size_t elem = 0;
		path++;
		while ((*path >= '0') && (*path <= '9')) {
			elem *= 10;
			elem += *path - '0';
			path++;
		}
		if (*path != ']')
			return NULL;

		if (elem >= Size())
			return NULL;

		jsonBase* jb = m_array[elem];
		if (!jb)
			return NULL;

		if (!*(++path))
			return jb;

		if (*path == '.')
			path++;

		switch (jb->GetType()) {
		case typeObject:	return ((json::jsonObject*)jb)->GetElementByPath(path);
		case typeArray:		return ((json::jsonArray*)jb)->GetElementByPath(path);
		default:
			if (!*path)		return jb;
			break;
		}
		return NULL;
	}
}
