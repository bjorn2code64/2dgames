#pragma once

#include "CommonControl.h"
#include "utils.h"

namespace CommonControls {

	class ToolBar : public CommonControl
	{
	public:
		ToolBar() {}
		~ToolBar() {}

		DWORD CreateAndShow(Window* parent, size_t nButtons, TBBUTTON* pButtons, const ImageList& il, int nCmdShow = SW_SHOW)
		{
			RETURN_IF_ERROR(CommonControl::Create(parent, TOOLBARCLASSNAME, NULL, WS_CHILD | TBSTYLE_FLAT));

			::SendMessage(*this, TB_SETIMAGELIST, (WPARAM)0, (LPARAM)(HIMAGELIST)il);
			::SendMessage(*this, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
			::SendMessage(*this, TB_ADDBUTTONS, (WPARAM)nButtons, (LPARAM)pButtons);
			::SendMessage(*this, TB_AUTOSIZE, 0, 0);

			Show(nCmdShow);
			return ERROR_SUCCESS;
		}
	};

	class ToolBarExt : public WindowExt, public ToolBar
	{
	public:
		ToolBarExt(int icons, bool large) : m_il(icons, large) {}

		~ToolBarExt() {
			for (auto& b : m_buttons) {
				free((void*)b.iString);
			}
		}

		DWORD Init(Window* pParent)
		{
			RETURN_IF_ERROR(CreateAndShow(pParent, m_buttons.size(), &m_buttons[0], m_il, SW_SHOW));

			return ERROR_SUCCESS;
		}

		void AddButton(int nCmd, LPCWSTR szText, int nIconID) {
			TBBUTTON tb;
			ZeroMemory(&tb, sizeof(TBBUTTON));
			tb.iBitmap = m_il.AddIcon(nIconID);
			tb.idCommand = nCmd;
			tb.fsState = TBSTATE_ENABLED;
			tb.fsStyle = BTNS_BUTTON;
			tb.dwData = 0;
			tb.iString = (INT_PTR)_wcsdup(szText);
			m_buttons.push_back(tb);
		}

		void AdjustClientRect(RECT* p) override {
			w32Rect r;
			::GetClientRect(*this, r);
			p->top += r.Height() + 1;
		}

		LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override
		{
			if (message == WM_SIZE)
				::SendMessage(*this, TB_AUTOSIZE, 0, 0);

			return 0;
		}

		void SetCheckByID(int id, BOOL check) {
			::SendMessage(*this, TB_CHECKBUTTON, (WPARAM)id, MAKELPARAM(check, 0));
		}

		void SetEnabledByID(int id, BOOL enabled) {
			::SendMessage(*this, TB_ENABLEBUTTON, (WPARAM)id, MAKELPARAM(enabled, 0));
		}

	protected:
		std::vector<TBBUTTON> m_buttons;
		ImageList m_il;
	};

}