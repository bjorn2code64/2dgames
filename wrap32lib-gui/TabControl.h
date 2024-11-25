#pragma once

#include "CommonControl.h"

namespace CommonControls {

	class TabControl : public CommonControl
	{
	public:
		TabControl() : m_child(NULL) {}

		DWORD CreateAndShow(Window* parent, DWORD dwStyles, Window* child, int nCmdShow = SW_SHOW) {
			m_child = child;
			RETURN_IF_ERROR(CommonControl::Create(parent, WC_TABCONTROL, NULL, dwStyles | WS_CHILD));

			::ShowWindow(*this, nCmdShow);
			return ERROR_SUCCESS;
		}

		int AppendTab(LPCWSTR sz, LPARAM lParam = 0) {
			TCITEM tcItem;
			memset(&tcItem, 0, sizeof(tcItem));
			tcItem.mask = TCIF_TEXT | TCIF_PARAM;
			tcItem.pszText = (LPWSTR)sz;
			tcItem.lParam = lParam;
			int nTabs = TabCtrl_GetItemCount(*this);
			TabCtrl_InsertItem(*this, nTabs, &tcItem);
			return nTabs;
		}

		void SetCurSelByData(LPARAM lParam) {
			TCITEM tcItem;
			memset(&tcItem, 0, sizeof(tcItem));
			tcItem.mask = TCIF_PARAM;

			int nTabs = TabCtrl_GetItemCount(*this);
			for (int i = 0; i < nTabs; i++) {
				TabCtrl_GetItem(*this, i, &tcItem);
				if (tcItem.lParam == lParam) {
					TabCtrl_SetCurSel(*this, i);
				}
			}
		}

		LPARAM GetCurSelData() {
			int nSel = TabCtrl_GetCurSel(*this);
			if (nSel < 0)
				return 0;

			TCITEM tcItem;
			memset(&tcItem, 0, sizeof(tcItem));
			tcItem.mask = TCIF_PARAM;
			TabCtrl_GetItem(*this, nSel, &tcItem);
			return tcItem.lParam;
		}

		LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override
		{
			if ((message == WM_SIZE) && m_child) {
				w32Rect r;
				::GetClientRect(*this, r);
				TabCtrl_AdjustRect(*this, FALSE, &r);
				::SetWindowPos(*m_child, NULL, r.left, r.top, r.Width() - 1, r.Height() - 1, SWP_NOZORDER);
			}

			return 0;
		}
	protected:
		Window* m_child;
	};

}