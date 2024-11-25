#pragma once

#include "wrap32lib.h"

extern UINT GetUniqueAppMessage();

class AppMessage
{
public:
	AppMessage() { m_uMsg = GetUniqueAppMessage(); }
	AppMessage(UINT u) : m_uMsg(u) {}

	operator UINT() const { return m_uMsg; }
	const AppMessage& operator=(UINT u) { m_uMsg = u; return *this; }

	UINT GetAppMessage() const { return m_uMsg; }

protected:
	UINT m_uMsg;
};

