#pragma once

#include "CommonControl.h"
#include "utils.h"
#include "ResourceStrings.h"

namespace CommonControls {

	class StatusBar : public CommonControl
	{
	public:
		StatusBar(int nParts = 1) : CommonControl(WINDOW_FLAGS_DONTREGISTER) {
			m_pWidths = new int[m_nParts = nParts];
			for (int i = 1; i < m_nParts; i++)
				m_pWidths[i] = 100;	// default the sizes
		}
		~StatusBar() {
			if (m_pWidths)
				delete[] m_pWidths;
		}

		DWORD CreateAndShow(Window* parent, int nCmdShow = SW_SHOW) {
			RETURN_IF_ERROR(CommonControl::Create(parent, STATUSCLASSNAME, NULL, WS_CHILD | SBARS_SIZEGRIP, 1));
			AddWindowStyles(0, WS_EX_COMPOSITED);
			ResizeParts();
			Show(nCmdShow);
			return ERROR_SUCCESS;
		}

		BOOL SetPartWidth(int nIndex, int nWidth) {
			if ((nIndex <= 0) || (nIndex >= m_nParts))	return FALSE;
			m_pWidths[nIndex] = nWidth;
			ResizeParts();
			return TRUE;
		}

		void SetPartText(int nIndex, LPCWSTR sz) {
			SendMessage(*this, SB_SETTEXT, nIndex, LPARAM(sz));
			RedrawWindow();
		}

		BOOL OnSize(UINT_PTR, w32Rect& r) override {
			// We've been resized - update the parts
			ResizeParts();
			return FALSE;
		}

		void SetPartOwnerDraw(int nIndex, LPCWSTR sz) {
			SendMessage(*this, SB_SETTEXT, nIndex | SBT_OWNERDRAW, LPARAM(sz));
		}

	protected:
		void ResizeParts() {
			int nWidthTotal = 0;
			int i;
			for (i = 1; i < m_nParts; i++)
				nWidthTotal += m_pWidths[i];
			w32Rect r;
			GetClientRect(*this, r);
			m_pWidths[0] = r.Width() - nWidthTotal;	// make p[0] fill the space

			int* pRights = new int[m_nParts];
			int nRight = 0;
			for (i = 0; i < m_nParts - 1; i++)
			{
				nRight += m_pWidths[i];
				pRights[i] = nRight;
			}

			pRights[m_nParts - 1] = -1;
			::SendMessage(*this, SB_SETPARTS, (WPARAM)m_nParts, (LPARAM)pRights);
			delete[] pRights;
		}

	protected:
		int* m_pWidths;
		int m_nParts;
	};

#define STATUSBAR_OPTS_SHOWMENUPROMPTS	0x0001

	class StatusBarExt : public WindowExt
	{
	public:
		// opts: 0 | STATUSBAR_OPTS_SHOWMENUPROMPTS
		StatusBarExt(int nParts, WORD wOpts, int nDefaultMessageID = 0) : m_statusBar(nParts), m_wOpts(wOpts), m_defaultMessage(nDefaultMessageID) {
		}

		DWORD Init(Window* pParent) override {
			RETURN_IF_ERROR(m_statusBar.CreateAndShow(pParent, SW_SHOW));

			m_statusBar.SetPartText(0, m_defaultMessage);
			return ERROR_SUCCESS;
		}

		LRESULT WndProc(HWND, UINT message, WPARAM wParam, LPARAM lParam) override {
			if (m_wOpts & STATUSBAR_OPTS_SHOWMENUPROMPTS) {
				if (message == WM_MENUSELECT) {
					int nID = LOWORD(wParam);
					if (nID)
					{
						ResourceString menuText(nID);
						LPCWSTR sz;
						if (menuText.Get(sz))
							m_statusBar.SetPartText(0, sz);
						else
							m_statusBar.SetPartText(0, m_defaultMessage);	// no string found
					}
					else	// menu closed
						m_statusBar.SetPartText(0, m_defaultMessage);

					return 1;
				}
			}

			if (message == WM_SIZE)
				::SendMessage(m_statusBar, WM_SIZE, 0, 0);

			return 0;
		}

		int GetControlID() { return GetDlgCtrlID(m_statusBar); }

		void AdjustClientRect(RECT* p) override {
			w32Rect r;
			::GetClientRect(m_statusBar, r);
			p->bottom -= r.Height();
		}

		void SetPartText(int nIndex, LPCWSTR sz) {
			m_statusBar.SetPartText(nIndex, sz);
		}

		BOOL SetPartWidth(int nIndex, int nWidth) {
			return m_statusBar.SetPartWidth(nIndex, nWidth);
		}

		void SetPartOwnerDraw(int nIndex, LPCWSTR sz) {
			m_statusBar.SetPartOwnerDraw(nIndex, sz);
		}

		operator HWND() {
			return m_statusBar;
		}

	protected:
		WORD m_wOpts;
		StatusBar m_statusBar;
		ResourceString m_defaultMessage;
	};

}