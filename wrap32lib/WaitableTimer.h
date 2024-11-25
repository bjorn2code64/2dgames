#pragma once

#include "wrap32lib.h"

class WaitableTimer {
public:
	WaitableTimer() {
		m_h = ::CreateWaitableTimer(NULL, FALSE, NULL);
	}

	~WaitableTimer() {
		::CloseHandle(m_h);
	}

	void Start(int ms) {
		LARGE_INTEGER li;
		li.QuadPart = -ms * 10000LL;
		::SetWaitableTimer(m_h, &li, ms, NULL, NULL, TRUE);
	}

	void Stop() {
		::CancelWaitableTimer(m_h);
	}

	operator HANDLE() { return m_h;  }

protected:
	HANDLE m_h;
};