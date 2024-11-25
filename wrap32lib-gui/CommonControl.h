#pragma once

#include "Window.h"

#include <CommCtrl.h>

#include "DateTime.h"

namespace CommonControls {

	class CommonControl : public Window
	{
	public:
		CommonControl(WORD wFlags = 0) : Window(wFlags | WINDOW_FLAGS_DONTREGISTER) {}

		static BOOL LibInit();

		DWORD Attach(HWND hWnd, HWND hWndParent)
		{
			SetHWnd(hWnd);
			SetParent(hWndParent);
			return SubClass(TRUE);
		}

		DWORD AttachToDlgItem(HWND hWndDialog, int nDlgID)
		{
			HWND hWnd = GetDlgItem(hWndDialog, nDlgID);
			if (!hWnd)
				return GetLastError();
			SetHWnd(hWnd);

			SetParent(hWndDialog);
			return SubClass(TRUE);
		}

		DWORD Detach() {
			RETURN_IF_ERROR(SubClass(FALSE));
			SetHWnd(NULL);
			SetParent(NULL);
			return ERROR_SUCCESS;
		}

		virtual HBRUSH OnColorCtrlFromParent(HDC hDC) { return NULL; }

	protected:
		DWORD Create(Window* parent, LPCWSTR lpszClassName, LPCWSTR lpszWindowName, DWORD dwStyles, int nID = 0, DWORD dwExStyles = 0);
		DWORD SubClass(BOOL bDoit);
	};

	class ImageList	// Part of the common control library
	{
	public:
		ImageList(int initial, bool large) {
			m_hIL = ImageList_Create(
				large ? GetSystemMetrics(SM_CXICON) : GetSystemMetrics(SM_CXSMICON),
				large ? GetSystemMetrics(SM_CYICON) : GetSystemMetrics(SM_CYSMICON),
				ILC_MASK, initial, 0);
		}

		ImageList(int nX, int nY, int initial) {
			m_hIL = ImageList_Create(nX, nY, ILC_MASK, initial, 0);
		}

		~ImageList() {
			ImageList_Destroy(m_hIL);
		}

		int AddIcon(int nID) {
			HICON hIcon = LoadIcon(Window::GetInstance(), MAKEINTRESOURCE(nID));
			int ret = ImageList_ReplaceIcon(m_hIL, -1, hIcon);
			DestroyIcon(hIcon);
			return ret;
		}

		operator HIMAGELIST() const { return m_hIL; }

	protected:
		HIMAGELIST m_hIL;
	};

	class Button : public CommonControl
	{
	public:
		Button(WORD wFlags = 0) : CommonControl(wFlags) {}

		DWORD CreateAndShow(Window* parent, LPCWSTR szText, int nCmdShow = SW_SHOW) {
			RETURN_IF_ERROR(CommonControl::Create(parent, WC_BUTTON, szText, WS_CHILD));

			::ShowWindow(*this, nCmdShow);
			return ERROR_SUCCESS;
		}

		static const int m_nDefaultWidth = 75;
		static const int m_nDefaultHeight = 23;
	};

	class GroupBox : public Button
	{
	public:
		DWORD CreateAndShow(Window* parent, LPCWSTR szText, int nCmdShow = SW_SHOW) {
			RETURN_IF_ERROR(CommonControl::Create(parent, WC_BUTTON, szText, WS_CHILD | BS_GROUPBOX));

			::ShowWindow(*this, nCmdShow);
			return ERROR_SUCCESS;
		}
	};

	class CheckBox : public Button
	{
	public:
		CheckBox() : Button(WINDOW_FLAGS_MESSAGESFROMPARENT), m_value(FALSE) {}

		DWORD CreateAndShow(Window* parent, LPCWSTR szText, int nCmdShow = SW_SHOW) {
			RETURN_IF_ERROR(CommonControl::Create(parent, WC_BUTTON, szText, WS_CHILD | BS_CHECKBOX));

			::ShowWindow(*this, nCmdShow);
			return ERROR_SUCCESS;
		}

		void operator=(BOOL b) {
			m_value = b;
			Button_SetCheck(*this, b);
		}

		operator BOOL() {
			return m_value;
		}

		DWORD AttachToDlgItem(HWND hWnd, int nID) {
			RETURN_IF_ERROR(__super::AttachToDlgItem(hWnd, nID));
			SetFlag(WINDOW_FLAGS_MESSAGESFROMPARENT, TRUE);
			*this = m_value;
			return ERROR_SUCCESS;
		}

	protected:
		BOOL OnCommandControlFromParent(int nCode, int nID) override {
			if (nCode == BN_CLICKED) {
				m_value = !m_value;// Button_GetCheck(*this);
				Button_SetCheck(*this, m_value);
			}

			return __super::OnCommandControlFromParent(nCode, nID);
		}

	protected:
		BOOL m_value;
	};

	class RadioButton : public Button
	{
	public:
		RadioButton() : Button(WINDOW_FLAGS_MESSAGESFROMPARENT) {}

		DWORD CreateAndShow(Window* parent, LPCWSTR szText, int myVal, int* current, int nCmdShow = SW_SHOW) {
			RETURN_IF_ERROR(CommonControl::Create(parent, WC_BUTTON, szText, WS_CHILD | BS_RADIOBUTTON));

			m_myval = myVal;
			m_current = current;
			::ShowWindow(*this, nCmdShow);
			return ERROR_SUCCESS;
		}

		void operator=(BOOL b) {
			Button_SetCheck(*this, b);
			if (b)
				*m_current = m_myval;
		}

		operator BOOL() {
			return (*m_current == m_myval);
		}

		DWORD AttachToDlgItem(HWND hWnd, int nID, int myVal, int* current) {
			RETURN_IF_ERROR(__super::AttachToDlgItem(hWnd, nID));
			SetFlag(WINDOW_FLAGS_MESSAGESFROMPARENT, TRUE);
			m_myval = myVal;
			m_current = current;
			*this = (*m_current == m_myval);
			return ERROR_SUCCESS;
		}

	protected:
		BOOL OnCommandControlFromParent(int nCode, int nID) override {
			if (nCode == BN_CLICKED) {
				if (Button_GetCheck(*this)) {
					*m_current = m_myval;
				}
			}

			return __super::OnCommandControlFromParent(nCode, nID);
		}

	protected:
		int* m_current;
		int m_myval;
	};

	class Static : public CommonControl
	{
	public:
		DWORD CreateAndShow(Window* parent, LPCWSTR szText, int nCmdShow = SW_SHOW) {
			RETURN_IF_ERROR(CommonControl::Create(parent, WC_STATIC, szText, WS_CHILD));

			::ShowWindow(*this, nCmdShow);
			return ERROR_SUCCESS;
		}

		DWORD CreateAndShow(Window* parent, int nIconID, int nCmdShow = SW_SHOW) {
			wchar_t szIconName[8];
			_snwprintf_s(szIconName, 8, L"#%d", nIconID);

			RETURN_IF_ERROR(CommonControl::Create(parent, WC_STATIC, szIconName, WS_CHILD | SS_ICON));

			::ShowWindow(*this, nCmdShow);
			return ERROR_SUCCESS;
		}
	};

	class DateTimePicker : public CommonControl
	{
	public:
		DateTimePicker() : CommonControl(WINDOW_FLAGS_MESSAGESFROMPARENT) {}

		DWORD CreateAndShow(Window* parent, LPCWSTR szText, int nCmdShow = SW_SHOW) {
			RETURN_IF_ERROR(CommonControl::Create(parent, DATETIMEPICK_CLASS, szText, WS_CHILD | DTS_SHOWNONE));

			::ShowWindow(*this, nCmdShow);
			return ERROR_SUCCESS;
		}

		DWORD AttachToDlgItem(HWND hWnd, int nID) {
			RETURN_IF_ERROR(__super::AttachToDlgItem(hWnd, nID));
			SetFlag(WINDOW_FLAGS_MESSAGESFROMPARENT, TRUE);
			*this = m_dt;
			return ERROR_SUCCESS;
		}

		void operator=(DateTime dt) {
			if (m_dt.IsValid())
				DateTime_SetSystemtime(*this, GDT_VALID, (const PSYSTEMTIME)m_dt);
			else
				DateTime_SetSystemtime(*this, GDT_NONE, (const PSYSTEMTIME)m_dt);
		}
		
		operator const DateTime& () {
			return m_dt;
		}

	protected:
		BOOL OnNotifyFromParent(UINT nCode, LPNMHDR lpnmhdr) override {
			if (nCode == DTN_DATETIMECHANGE) {
				LPNMDATETIMECHANGE lpDTC = (LPNMDATETIMECHANGE)lpnmhdr;
				if (lpDTC->dwFlags == GDT_NONE)
					m_dt.Clear();
				else
					m_dt = ((LPNMDATETIMECHANGE)lpnmhdr)->st;
			}

			return __super::OnNotifyFromParent(nCode, lpnmhdr);
		}

	protected:
		DateTime m_dt;
	};
}

#include "ListBox.h"
#include "ListView.h"
#include "TreeView.h"
#include "Edit.h"
#include "ComboBox.h"
#include "StatusBar.h"
#include "TabControl.h"
#include "ToolBar.h"
#include "ToolTip.h"
