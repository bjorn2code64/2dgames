#pragma once

#include "wrap32lib.h"

class CriticalSection
{
public:
	CriticalSection() {	::InitializeCriticalSection(&m_cs);	}
	~CriticalSection() {	::DeleteCriticalSection(&m_cs);	}

	void Enter() {	::EnterCriticalSection(&m_cs);	}
	void Leave() {	::LeaveCriticalSection(&m_cs);	}

protected:
	CRITICAL_SECTION m_cs;
};

class CSLocker	// lock a CS during life of this object - handy for functions with multiple return points
{
public:
	CSLocker(CriticalSection& cs) : m_cs(cs) {	m_cs.Enter();	}
	~CSLocker() {	m_cs.Leave();	}

protected:
	CriticalSection& m_cs;
};
