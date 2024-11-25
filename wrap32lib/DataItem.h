#pragma once

#include "wrap32lib.h"
#include "StringValidator.h"
#include "unicodemultibyte.h"

class DataItem
{
public:
	enum class type {
		type_unknown,
		type_bool,
		type_uint16,
		type_uint32,
		type_float,
		type_double,
		type_string,
		type_uint8
	};

	DataItem(type t = type::type_unknown, LPCWSTR sz = NULL) : m_type(t), m_subscribed(false) {
		if (sz)
			SetFromString(sz);
		else
			ZeroMemory(&m_val, sizeof(m_val));
	}

	bool operator>(const DataItem& rhs) {
		if (m_type != rhs.m_type)
			return false;

		switch (m_type) {
		case type::type_uint8:	return m_val.m_uint8 > rhs.m_val.m_uint8;
		case type::type_uint16:	return m_val.m_uint16 > rhs.m_val.m_uint16;
		case type::type_uint32:	return m_val.m_uint32 > rhs.m_val.m_uint32;
		case type::type_float:	return m_val.m_float > rhs.m_val.m_float;
		case type::type_double:return m_val.m_float > rhs.m_val.m_float;
		default: break;
		}
		return false;
	}

	bool operator<=(const DataItem& rhs) {
		return !operator>(rhs);
	}

	bool operator<(const DataItem& rhs) {
		if (m_type != rhs.m_type)
			return false;

		switch (m_type) {
		case type::type_uint8:	return m_val.m_uint8 < rhs.m_val.m_uint8;
		case type::type_uint16:	return m_val.m_uint16 < rhs.m_val.m_uint16;
		case type::type_uint32:	return m_val.m_uint32 < rhs.m_val.m_uint32;
		case type::type_float:	return m_val.m_float < rhs.m_val.m_float;
		case type::type_double:	return m_val.m_double < rhs.m_val.m_double;
		case type::type_string:	return strcmp(m_string.c_str(), rhs.m_string.c_str()) < 0;
		default: break;
		}
		return false;
	}

	bool operator>=(const DataItem& rhs) {
		return !operator<(rhs);
	}

	bool operator==(const DataItem& rhs) {
		if (m_type != rhs.m_type)
			return false;

		switch (m_type) {
		case type::type_uint8:	return m_val.m_uint8 == rhs.m_val.m_uint8;
		case type::type_uint16:	return m_val.m_uint16 == rhs.m_val.m_uint16;
		case type::type_uint32:	return m_val.m_uint32 == rhs.m_val.m_uint32;
		case type::type_float:	return m_val.m_float == rhs.m_val.m_float;
		case type::type_double:	return m_val.m_double == rhs.m_val.m_double;
		case type::type_string:	return strcmp(m_string.c_str(), rhs.m_string.c_str()) == 0;
		default: break;
		}
		return false;
	}

	bool operator!=(const DataItem& rhs) {
		return !operator==(rhs);
	}

	bool SetFromString(LPCWSTR sz) {
		switch (m_type) {
		case type::type_bool: {
			if (!wcscmp(sz, L"0"))
				m_val.m_bool = false;
			else if (!wcscmp(sz, L"1"))
				m_val.m_bool = true;
			else
				return false;
			break;
		}

		case type::type_uint8: {
			LPWSTR szEnd;
			unsigned long ul = wcstoul(sz, &szEnd, 10);
			if (*szEnd)			return false;
			if (ul > 255)		return false;
			m_val.m_uint8 = (uint8_t)ul;
			break;
		}

		case type::type_uint16: {
			LPWSTR szEnd;
			unsigned long ul = wcstoul(sz, &szEnd, 10);
			if (*szEnd)			return false;
			if (ul > 65335)		return false;
			m_val.m_uint16 = (uint16_t)ul;
			break;
		}

		case type::type_uint32: {
			LPWSTR szEnd;
			unsigned long ul = wcstoul(sz, &szEnd, 10);
			if (*szEnd)			return false;
			m_val.m_uint32 = ul;
			break;
		}

		case type::type_float: {
			LPWSTR szEnd;
			float f = wcstof(sz, &szEnd);
			if (*szEnd)			return false;
			m_val.m_float = f;
			break;
		}
		case type::type_double: {
			LPWSTR szEnd;
			double f = wcstod(sz, &szEnd);
			if (*szEnd)			return false;
			m_val.m_double = f;
			break;
		}
		case type::type_string: {
			m_string = UnicodeMultibyte(sz);
			break;
		}
		default:
			return false;
		}

		return true;
	}

	const DataItem& operator=(const DataItem& value) {
		m_val = value.m_val;
		m_string = value.m_string;
		m_type = value.m_type;
		// don't copy the subscribed flag
		return *this;
	}

	const DataItem& operator=(bool value) {
		m_val.m_bool = value;
		m_type = type::type_bool;
		return *this;
	}

	operator bool() const {
		return (m_type == type::type_bool) ? m_val.m_bool : false;
	}

	const DataItem& operator=(uint8_t value) {
		m_val.m_uint8 = value;
		m_type = type::type_uint8;
		return *this;
	}

	operator uint8_t() const {
		return (m_type == type::type_uint8) ? m_val.m_uint8 : 0;
	}

	const DataItem& operator=(uint16_t value) {
		m_val.m_uint16 = value;
		m_type = type::type_uint16;
		return *this;
	}

	operator uint16_t() const {
		return (m_type == type::type_uint16) ? m_val.m_uint16 : 0;
	}

	const DataItem& operator=(uint32_t value) {
		m_val.m_uint32 = value;
		m_type = type::type_uint32;
		return *this;
	}

	operator uint32_t() const {
		return (m_type == type::type_uint32) ? m_val.m_uint32 : 0;
	}

	const DataItem& operator=(float value) {
		m_val.m_float = value;
		m_type = type::type_float;
		return *this;
	}

	operator float() const {
		return (m_type == type::type_float) ? m_val.m_float : 0.0f;
	}

	const DataItem& operator=(double value) {
		m_val.m_double = value;
		m_type = type::type_double;
		return *this;
	}

	operator double() const {
		return (m_type == type::type_double) ? m_val.m_double : 0.0;
	}

	const DataItem& operator=(LPCSTR sz) {
		m_string = sz;
		m_type = type::type_string;
		return *this;
	}

	operator LPCSTR() const {
		return (m_type == type::type_string) ? m_string.c_str() : NULL;
	}

	void GetAsString(LPWSTR buff, int nLen) const {
		switch (m_type) {
		case type::type_bool:
			wcscpy_s(buff, nLen, m_val.m_bool ? L"1" : L"0");
			break;
		case type::type_uint8:
			swprintf_s(buff, nLen, L"%d", (int)m_val.m_uint8);
			break;
		case type::type_uint16:
			swprintf_s(buff, nLen, L"%d", (int)m_val.m_uint16);
			break;
		case type::type_uint32:
			swprintf_s(buff, nLen, L"%d", (int)m_val.m_uint32);
			break;
		case type::type_float:
			swprintf_s(buff, nLen, L"%.15g", m_val.m_float);
			break;
		case type::type_double:
			swprintf_s(buff, nLen, L"%.15g", m_val.m_double);
			break;
		case type::type_string:
			wcscpy_s(buff, nLen, UnicodeMultibyte(m_string));
			break;
		default:
			wcscpy_s(buff, nLen, L"?");
			break;
		}
	}

	void GetAsString(std::wstring& ws) const {
		switch (m_type) {
		case type::type_bool:
			ws = m_val.m_bool ? L"1" : L"0";
			break;
		case type::type_uint8:
			ws = std::to_wstring(m_val.m_uint8);
			break;
		case type::type_uint16:
			ws = std::to_wstring(m_val.m_uint16);
			break;
		case type::type_uint32:
			ws = std::to_wstring(m_val.m_uint32);
			break;
		case type::type_float:
			ws = std::to_wstring(m_val.m_float);
			break;
		case type::type_double:
			ws = std::to_wstring(m_val.m_double);
			break;
		case type::type_string:
			ws = UnicodeMultibyte(m_string);
			break;
		default:
			ws = L"";
			break;
		}
	}

	type GetType() const { return m_type; }

	LPCWSTR GetTypeAsString() const {
		static LPCWSTR names[] = {
			L"Unknown",
			L"Bit",
			L"Unsigned Short",
			L"Unsigned Int",
			L"Float",
			L"Double",
			L"String"
		};
		return names[(int)m_type];
	}

	bool IsSubscribed() const { return m_subscribed;  }
	void Subscribe(bool b = true) { m_subscribed = b;  }

protected:
	type m_type;
	bool m_subscribed;

	union val {
		bool m_bool;
		uint8_t m_uint8;
		uint16_t m_uint16;
		uint32_t m_uint32;
		float m_float;
		double m_double;
	} m_val;
	std::string m_string;
};
