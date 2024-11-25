#pragma once

#include "wrap32lib.h"
#include <process.h>

#include "Event.h"
#include "TimerQueue.h"
#include "TimerEventLoop.h"

class Thread
{
public:
	Thread() : m_h(NULL), m_evStop(FALSE), m_dwID(0)	{}
	~Thread(void)	{	if (Running()) MessageBox(NULL, L"Error: Call Stop() in derived destructor", L"Thread", MB_OK);		}	// Call Stop() in derived class destructors!!!

	BOOL Start() {
		if (m_h)			return FALSE;	// already running

		m_evStop.Reset();
		m_h = (void*)_beginthreadex(NULL, 0, ThreadProcStatic, this, 0, &m_dwID);
		return m_h != NULL;
	}

	virtual void Stop(bool bWait = true) {	// Pass false for a quick return and you can call WaitForStopped() later - useful for stopping many threads quickly
		m_evStop.Set();
		if (bWait)
			WaitForStopped();
	}

	void WaitForStopped() {
		while (Running()) {
			Sleep(100);
		}
	}

	bool Running() {
		DWORD dwExitCode;
		return (m_h != NULL) && GetExitCodeThread(m_h, &dwExitCode) && (dwExitCode == STILL_ACTIVE);
	}

	BOOL Allocated()	{	return m_h != NULL;			}	// May be starting up?
	bool ShouldStop()	{	return m_evStop.Wait(0);	}

protected:
	bool WaitForStop(DWORD dwMS = INFINITE)	{	return m_evStop.Wait(dwMS);	}

	void ThreadWrapper() {
		ThreadProc();
		CloseHandle(m_h);	// _beginthreadex() leaves this for us to do
		m_h = NULL;
	}

	virtual void ThreadProc() = 0;	// override this - check for m_evStop and quit if it gets set
	static unsigned int WINAPI ThreadProcStatic(LPVOID lpParameter)	{
		((Thread*)lpParameter)->ThreadWrapper();
		return 0;
	}

protected:
	HANDLE m_h;
	unsigned int m_dwID;
	Event m_evStop;
};

// An efficient thread framework that sleeps until events happen or timers are set.
class EventThread : public Thread, public TimerEventLoop
{
public:
	EventThread() : TimerEventLoop(m_evStop) {
	}

protected:
	void ThreadProc() final {	// Stop this being overridden any further, user StartupShutdown
		ThreadStartup();
		TELRun();
		ThreadShutdown();
	}


	virtual void ThreadStartup() {}
	virtual void ThreadShutdown() {}
};
