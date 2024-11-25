#pragma once

#include "wrap32lib.h"

#include <vector>
#include <functional>

static VOID CALLBACK tcallback(
	_In_ PVOID   lpParameter,
	_In_ BOOLEAN TimerOrWaitFired);

class TimerQueue
{
public:
	class Timer
	{
	public:
		Timer(HANDLE hTimerQueue, std::function<bool(LPVOID, BOOLEAN)> callback, LPVOID p) :
			m_hTimerQueue(hTimerQueue),
			m_callback(callback),
			m_p(p),
			m_hTimer(INVALID_HANDLE_VALUE)
		{
		}

		~Timer() {
			Stop();
		}

		void Start(DWORD dwTime, DWORD dwPeriod = 0) {
			::CreateTimerQueueTimer(&m_hTimer, m_hTimerQueue, tcallback, this, dwTime, dwPeriod, WT_EXECUTEDEFAULT);
		}

		void Stop() {
			if (m_hTimer != INVALID_HANDLE_VALUE) {
				(void)::DeleteTimerQueueTimer(m_hTimerQueue, m_hTimer, NULL);
				m_hTimer = INVALID_HANDLE_VALUE;
			}
		}

		void Callback(BOOLEAN TimerOrWaitFired)
		{
			if (m_callback(m_p, TimerOrWaitFired))
				Stop();
		}

	protected:
		HANDLE m_hTimerQueue;
		HANDLE m_hTimer;
		std::function<bool(LPVOID, BOOLEAN)> m_callback;
		LPVOID m_p;
	};

	TimerQueue() {
		m_hTQ = ::CreateTimerQueue();
	}

	~TimerQueue() {
		for (auto& timer : m_timers)
			delete timer;

		(void)::DeleteTimerQueueEx(m_hTQ, NULL);	// don't wait for stuff
	}

	Timer* CreateTimer(std::function<bool(LPVOID, BOOLEAN)> callback, LPVOID p) {
		Timer* t = new Timer(m_hTQ, callback, p);
		m_timers.push_back(t);
		return t;
	}

protected:
	HANDLE m_hTQ;
	std::vector<Timer*> m_timers;
};

static VOID CALLBACK tcallback(
	_In_ PVOID   lpParameter,
	_In_ BOOLEAN TimerOrWaitFired) {
	((TimerQueue::Timer*)lpParameter)->Callback(TimerOrWaitFired);
}
