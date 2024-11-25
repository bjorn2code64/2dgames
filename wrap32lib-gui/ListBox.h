#pragma once

#include "CommonControl.h"

#include <string>

namespace CommonControls {

	class ListBox : public CommonControl
	{
	public:
		DWORD CreateAndShow(Window* parent, int nCmdShow = SW_SHOW) {
			RETURN_IF_ERROR(__super::Create(parent, WC_LISTBOX, NULL, WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY));

			::ShowWindow(*this, nCmdShow);
			return ERROR_SUCCESS;
		}

		int AddString(LPCWSTR sz, LPARAM data = 0, bool select = false) {
			int index = ListBox_AddString(*this, sz);
			ListBox_SetItemData(*this, index, data);
			if (select)
				ListBox_SetCurSel(*this, index);
			return index;
		}

		bool GetCurSelWString(std::wstring& ret) {
			int index = ListBox_GetCurSel(*this);
			if (index == LB_ERR)
				return false;

			size_t len = ListBox_GetTextLen(*this, index);
			wchar_t* p = new wchar_t[len + 1];
			ListBox_GetText(*this, index, p);
			ret = p;
			delete[] p;

			return true;
		}

		LPARAM GetCurSelData() {
			int index = ListBox_GetCurSel(*this);
			if (index == LB_ERR)
				return 0;

			return ListBox_GetItemData(*this, index);
		}
	};
}