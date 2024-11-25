#pragma once

#include "wrap32lib.h"
#include <regex>

class StringValidator
{
protected:
	LPCWSTR m_ValidEmailRegex = L"(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+";
	LPCWSTR m_ValidIpAddressRegex = L"^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$";
//  NEW RFC allows hostnames to start with a number - doesn't look good though as you can type 1.2.3 which isn't great???
//	LPCWSTR m_ValidHostnameRegexNEW = L"^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])$";
	LPCWSTR m_ValidHostnameRegex = L"^(([a-zA-Z]|[a-zA-Z][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z]|[A-Za-z][A-Za-z0-9\\-]*[A-Za-z0-9])$";
public:
	StringValidator(LPCWSTR sz) : m_sz(sz) {
	}

	enum class type {
		type_uint,
		type_int,
		type_hexint,
		type_double,
		type_varname,	// leading letter then letters and numbers and underscore
		type_email,
		type_hostname
	};

	BOOL IsValid(LPCWSTR regex) {
		return std::regex_match(m_sz, std::wregex(regex));
	}

	BOOL IsValid(type t) {
		LPCWSTR sz = m_sz;

		switch (t) {
		case type::type_uint:
			if (!ConsumeDigits(&sz, TRUE))	// required
				return FALSE;
			break;
		case type::type_int:
			ConsumePlusMinus(&sz, FALSE);	// optional
			if (!ConsumeDigits(&sz, TRUE))	// required
				return FALSE;
			break;
		case type::type_hexint:
			ConsumePlusMinus(&sz, FALSE);	// optional
			if (!ConsumeHexDigits(&sz, TRUE))	// required
				return FALSE;
			break;
		case type::type_double: {
			BOOL bMust = FALSE;
			ConsumePlusMinus(&sz, FALSE);	// optional
			bMust |= ConsumeDigits(&sz, TRUE);
			Consume(&sz, L".", FALSE);
			bMust |= ConsumeDigits(&sz, TRUE);
			if (!bMust)
				return FALSE;
		}
		break;
		case type::type_varname:
			if (!ConsumeAlpha(&sz, FALSE))	// required
				return FALSE;

			if (!ConsumeAlphaUnderscore(&sz, TRUE))	// optional
				return FALSE;
			break;
		case type::type_email:
			return std::regex_match(m_sz, std::wregex(m_ValidEmailRegex));

		case type::type_hostname:
			return std::regex_match(m_sz, std::wregex(m_ValidIpAddressRegex)) || std::regex_match(m_sz, std::wregex(m_ValidHostnameRegex));
		}

		return *sz == '\0';
	}

protected:
	BOOL ConsumePlusMinus(LPCWSTR* pp, BOOL bMultiple) {
		return Consume(pp, L"+-", bMultiple);
	}

	BOOL ConsumeDigits(LPCWSTR* pp, BOOL bMultiple) {
		return Consume(pp, L"0123456789", bMultiple);
	}

	BOOL ConsumeHexDigits(LPCWSTR* pp, BOOL bMultiple) {
		return Consume(pp, L"0123456789ABCDEFabcdef", bMultiple);
	}

	BOOL ConsumeAlpha(LPCWSTR* pp, BOOL bMultiple) {
		return Consume(pp, L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", bMultiple);
	}

	BOOL ConsumeAlphaUnderscore(LPCWSTR* pp, BOOL bMultiple) {
		return Consume(pp, L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", bMultiple);
	}

	BOOL Consume(LPCWSTR* pp, LPCWSTR chars, BOOL bMultiple) {
		BOOL bFound = FALSE;
		while (*(*pp) && wcschr(chars, *(*pp))) {
			(*pp)++;
			bFound = TRUE;
			if (!bMultiple)
				break;
		}
		return bFound;
	}

protected:
	LPCWSTR m_sz;
};
