#pragma once

#include "wrap32lib.h"
#include <time.h>
#include <string>
#include <sstream>

class DateTime
{
public:
	DateTime() {
		Clear();
	}

	DateTime(ULONGLONG ull) {
		*this = ull;
	}

	DWORD Set(FILETIME ft) {
		if (!::FileTimeToSystemTime(&ft, &m_st))
			return ::GetLastError();

		if (!::SystemTimeToTzSpecificLocalTime(NULL, &m_st, &m_st))
			return ::GetLastError();

		return ERROR_SUCCESS;
	}

	void SetNow() {
		::GetLocalTime(&m_st);
	}

	bool Set(WORD y, BYTE m, BYTE d, BYTE h, BYTE M, BYTE s) {
		Clear();
		m_st.wYear = y;
		m_st.wMonth = m;
		m_st.wDay = d;
		m_st.wHour = h;
		m_st.wMinute = M;
		m_st.wSecond = s;
		return IsValid();
	}

	void GetDateTime(wchar_t* buff, int len, char delim1 = '/', char delim2 = ':', char sep = ' ') const {
		if (m_st.wYear == 0)
			wcscpy_s(buff, len, L"?");
		else {
			wchar_t mask[32];
			_snwprintf_s(mask, 32, L"%%04d%c%%02d%c%%02d%c%%02d%c%%02d%c%%02d", delim1, delim1, sep, delim2, delim2);
			swprintf_s(buff, len, mask, //L"%04d/%02d/%02d %02d:%02d:%02d",
				m_st.wYear, m_st.wMonth, m_st.wDay,
				m_st.wHour, m_st.wMinute, m_st.wSecond);
		}
	}

	void GetDate(wchar_t* buff, int len) const {
		if (m_st.wYear == 0)
			wcscpy_s(buff, len, L"?");
		else
			swprintf_s(buff, len, L"%04d/%02d/%02d",
				m_st.wYear, m_st.wMonth, m_st.wDay);
	}

	void GetTime(wchar_t* buff, int len) const {
		if (m_st.wYear == 0)
			wcscpy_s(buff, len, L"?");
		else
			swprintf_s(buff, len, L"%02d:%02d:%02d",
				m_st.wHour, m_st.wMinute, m_st.wSecond);
	}

	void GetTimeWithMS(wchar_t* buff, int len) const {
		swprintf_s(buff, len, L"%02d:%02d:%02d:%03d",
			m_st.wHour, m_st.wMinute, m_st.wSecond, m_st.wMilliseconds);
	}

	WORD GetYear() const { return m_st.wYear;  }	// Full year e.g. 2020
	WORD GetMonth() const { return m_st.wMonth; }	// Jan=1, Dec=12
	WORD GetDay() const { return m_st.wDay; }		// 1->31
	WORD GetHour() const { return m_st.wHour; }		// 0->23
	WORD GetMinute() const { return m_st.wMinute; }	// 0->59
	WORD GetSecond() const { return m_st.wSecond; }	// 0->59

	void SetHour(WORD w) { m_st.wHour = w; }
	void SetMinute(WORD w) { m_st.wMinute = w; }
	void SetSecond(WORD w) { m_st.wSecond = w;  }

	bool Parse(const char* sz, char delim1 = '/', char delim2 = '.', char sep = ' ') {
		char mask[32];
		_snprintf_s(mask, 32, "%%d%c%%d%c%%d%c%%d%c%%d%c%%d", delim1, delim1, sep, delim2, delim2);
		int y, m, d, h, M, s;
		if (sscanf_s(sz, mask, &y, &m, &d, &h, &M, &s) != 6)
			return false;
		Clear();
		m_st.wYear = (WORD)y;
		m_st.wMonth = (WORD)m;
		m_st.wDay = (WORD)d;
		m_st.wHour = (WORD)h;
		m_st.wMinute = (WORD)M;
		m_st.wSecond = (WORD)s;
		return true;
	}

	void Clear() {
		::ZeroMemory(&m_st, sizeof(m_st));
	}

	bool IsLeapYear() const {
		if ((m_st.wYear % 4) == 0 && ((m_st.wYear % 100) != 0 || (m_st.wYear % 400) == 0))
			return true;
		return false;
	}

	bool IsValid() const {
		static WORD nMonthDays[12] = {
		   31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
		};

		// Check values
		if (m_st.wMonth == 0 || m_st.wMonth > 12 ||
			m_st.wDay == 0 || m_st.wDay > nMonthDays[m_st.wMonth - 1] ||
			m_st.wHour > 23 ||
			m_st.wMinute > 59 ||
			m_st.wSecond > 59 ||
			m_st.wMilliseconds > 999) {
			return false;
		}

		// February 29 only allowed on leap years
		if (m_st.wMonth == 2 && m_st.wDay == 29 && !IsLeapYear())
			return false;

		return true;
	}

	operator ULONGLONG() const {
		FILETIME ft;
		if (!::SystemTimeToFileTime(&m_st, &ft))
			return 0;
		ULARGE_INTEGER uli;
		memcpy(&uli, &ft, sizeof(ULARGE_INTEGER));
		return uli.QuadPart;
	}

	const DateTime& operator=(ULONGLONG ull) {
		ULARGE_INTEGER uli;
		uli.QuadPart = ull;
		FILETIME ft;
		memcpy(&ft, &uli, sizeof(ULARGE_INTEGER));
		::FileTimeToSystemTime(&ft, &m_st);
		return *this;
	}

	const DateTime& operator=(const SYSTEMTIME& st) {
		m_st = st;
		return *this;
	}

	bool operator<(const DateTime& rhs) const {
		return ULONGLONG(*this) < ULONGLONG(rhs);
	}

	bool operator>(const DateTime& rhs) const {
		return ULONGLONG(*this) > ULONGLONG(rhs);
	}

	bool operator==(const DateTime& rhs) const {
		return ULONGLONG(*this) == ULONGLONG(rhs);
	}

	// Seconds difference
	int DiffSeconds(const DateTime& rhs) const {
		ULONGLONG div = 10000000LL;
		ULONGLONG diff = ULONGLONG(*this) - ULONGLONG(rhs);
		return int(diff / div);
	}

	int DiffMinutes(const DateTime& rhs) const {
		ULONGLONG div = 600000000LL;
		ULONGLONG diff = ULONGLONG(*this) - ULONGLONG(rhs);
		return int(diff / div);
	}

	void ClearMS() {
		m_st.wMilliseconds = 0;
	}

	operator PSYSTEMTIME() {
		return &m_st;
	}

	// Can't add months and years, e.g. add one month the 31st jan? Add 1 year to 29th Feb?
	void Add(int days, int hours, int mins, int secs) {
		ULONGLONG ull = *this;
		ULONGLONG ullPeriod = 10000000LL; // secs
		ull += (ULONGLONG)secs * ullPeriod;
		ullPeriod *= 60;	// mins
		ull += (ULONGLONG)mins * ullPeriod;
		ullPeriod *= 60;	// hours
		ull += (ULONGLONG)hours * ullPeriod;
		ullPeriod *= 24;	// days
		ull += (ULONGLONG)days * ullPeriod;
		*this = ull;
	}

	void SetTime(time_t t, bool adjustToLocal) {
		struct tm tm;
		if (adjustToLocal) {
			localtime_s(&tm, &t);
		}
		else {
			gmtime_s(&tm, &t);
		}
		m_st.wYear = (WORD)tm.tm_year + 1900;
		m_st.wMonth = (WORD)tm.tm_mon + 1;
		m_st.wDay = (WORD)tm.tm_mday;
		m_st.wHour = (WORD)tm.tm_hour;
		m_st.wMinute = (WORD)tm.tm_min;
		m_st.wSecond = (WORD)tm.tm_sec;
		m_st.wMilliseconds = 0;
		m_st.wDayOfWeek = (WORD)tm.tm_wday;
	}

protected:
	SYSTEMTIME m_st;
};

