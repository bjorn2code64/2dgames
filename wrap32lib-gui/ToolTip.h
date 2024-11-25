#pragma once

#include <StringValidator.h>
#include <StringParser.h>
#include <DataItem.h>

#include "CommonControl.h"

#include <string>
#include <sstream>

namespace CommonControls {
	// Doesn't yet work :(
	class ToolTip : public CommonControl
	{
	public:
		DWORD CreateAndShow(Window* parent) {
			RETURN_IF_ERROR(Window::Create(parent, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, nullptr, 0, WS_EX_TOPMOST));
//			RETURN_IF_ERROR(__super::Create(parent, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, 0, WS_EX_TOPMOST));
			TOOLINFO ti = { 0 };
			ti.cbSize = sizeof(TOOLINFO);
			ti.uFlags = TTF_SUBCLASS;
			ti.hwnd = *parent;
			ti.hinst = GetInstance();
			ti.lpszText = (LPWSTR)L"This is your tooltip string.";
			::GetClientRect(*parent, &ti.rect);
			SendMessage(*this, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
			SendMessage(*this, TTM_ACTIVATE, TRUE, 0);
			return ERROR_SUCCESS;
		}

		void OnSize(w32Rect& r) {
			if (HWND(*this) == NULL) return;
			TOOLINFO ti = { 0 };
			ti.cbSize = sizeof(TOOLINFO);
			ti.hwnd = *GetParent();
			::SendMessage(*this, TTM_GETTOOLINFO, 0, (LPARAM) & ti);
			ti.rect = r;
			::SendMessage(*this, TTM_NEWTOOLRECT, 0, (LPARAM)&ti);
		}
	};
}
