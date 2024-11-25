#pragma once

#include "wrap32lib.h"

class NotifyTarget
{
public:
	virtual LRESULT NotifySend(WPARAM wParam, LPARAM lParam = 0) { return 0; }
	virtual void NotifyPost(WPARAM wParam, LPARAM lParam = 0) {}
};
