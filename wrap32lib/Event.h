#pragma once

#include "wrap32lib.h"

class Event
{
public:
	Event(BOOL bAutoReset = TRUE) {
		m_h = ::CreateEvent(NULL, !bAutoReset, FALSE, NULL);
	}

	~Event() {
		::CloseHandle(m_h);
	}

	operator HANDLE() {
		return m_h;
	}

	void Set() {
		::SetEvent(m_h);
	}

	void Reset() {
		::ResetEvent(m_h);
	}

	bool Wait(UINT dwMS = INFINITE) {	// true if event was seen, false if it timed out (or other error)
		return ::WaitForSingleObject(m_h, dwMS) == WAIT_OBJECT_0;
	}

protected:
	HANDLE m_h;
};