#pragma once

#include "wrap32lib.h"
#include "Event.h"
#include "TimerQueue.h"

class TimerEventLoop
{
protected:
	static bool setEventProc(PVOID lpParameter, BOOLEAN /*TimerOrWaitFired*/) {
		((Event*)lpParameter)->Set();
		return false;	// don't stop
	}

public:
	TimerEventLoop(Event& evStop) {
		TELAddEvent(evStop, [this]() {
			return true;
			});
	}

	~TimerEventLoop() {
		for (auto timer : m_timerEvents) {
			delete timer;
		}
	}

	void TELAddEvent(Event& ev, std::function<bool()> func) {
		m_handles.push_back(ev);
		m_functions.push_back(func);
	}

	TimerQueue::Timer* TELAddTimer(std::function<bool()> func) {
		Event* evTimer = new Event;
		m_timerEvents.push_back(evTimer);
		TimerQueue::Timer* timer = m_tq.CreateTimer(setEventProc, evTimer);
		TELAddEvent(*evTimer, func);
		return timer;
	}

	void TELRun() {
		bool done = false;
		while (!done) {
			DWORD ret = WaitForMultipleObjects((DWORD)m_handles.size(), &m_handles[0], FALSE, INFINITE);
			if (ret == WAIT_FAILED) {
				OnWaitFailed();
				continue;
			}
			else if (ret == WAIT_TIMEOUT) {
				OnTimeout();
				continue;
			}

			// Dispatch
			done = m_functions.at(ret - WAIT_OBJECT_0)();
		}
	}

	virtual void OnTimeout() {}
	virtual void OnWaitFailed() {}

protected:
	std::vector<HANDLE> m_handles;
	std::vector<std::function<bool()>> m_functions;	// return true to quit

	TimerQueue m_tq;
	std::vector<Event*> m_timerEvents;
};