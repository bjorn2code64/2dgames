#pragma once

#include "CommonControl.h"

namespace CommonControls {

	class ComboBox : public CommonControl
	{
	public:
		DWORD CreateAndShow(Window* parent, DWORD dwStyles = 0, int nCmdShow = SW_SHOW) {
			RETURN_IF_ERROR(CommonControl::Create(parent, WC_COMBOBOX, NULL, WS_CHILD | dwStyles));

			::ShowWindow(*this, nCmdShow);
			return ERROR_SUCCESS;
		}

		int AddString(LPCWSTR sz, LPARAM lParam = NULL) {
			int index = ComboBox_AddString(*this, sz);
			ComboBox_SetItemData(*this, index, lParam);
			return index;
		}

		LPARAM GetCurSelData() {
			int index = ComboBox_GetCurSel(*this);
			return ComboBox_GetItemData(*this, index);
		}

		void SetCurSelByData(LPARAM lParam) {
			int count = ComboBox_GetCount(*this);
			for (int i = 0; i < count; i++) {
				if (ComboBox_GetItemData(*this, i) == lParam) {
					ComboBox_SetCurSel(*this, i);
				}
			}
		}

		void SetCurSelByWString(LPCWSTR wsz) {
			int count = ComboBox_GetCount(*this);
			wchar_t buff[256];
			for (int i = 0; i < count; i++) {
				ComboBox_GetLBText(*this, i, buff);
				if (!wcscmp(buff, wsz)) {
					ComboBox_SetCurSel(*this, i);
				}
			}
		}

		bool GetSelected(std::wstring& ret) {
			int index = ComboBox_GetCurSel(*this);
			if (index < 0)
				return false;
			int len = ComboBox_GetLBTextLen(*this, index);
			wchar_t* sz = new wchar_t[(size_t)len + 1];
			ComboBox_GetLBText(*this, index, sz);
			ret = sz;
			delete[] sz;
			return true;
		}
	};

	class ComboBoxLParam : public ComboBox
	{
	public:
		ComboBoxLParam() : m_value(0) {}

		const ComboBoxLParam& operator=(LPARAM n) {
			m_value = n; ComboBox::SetCurSelByData(n);	return *this;
		}

		DWORD AttachToDlgItem(HWND hWnd, int nID) {
			RETURN_IF_ERROR(__super::AttachToDlgItem(hWnd, nID));
			SetFlag(WINDOW_FLAGS_MESSAGESFROMPARENT, TRUE);
			ComboBox::SetCurSelByData(m_value);
			return ERROR_SUCCESS;
		}

		operator LPARAM() {
			return m_value;
		}

		int AddString(LPCWSTR sz, LPARAM lParam) {
			int nRet = __super::AddString(sz, lParam);
			if (lParam == m_value) {
				ComboBox_SetCurSel(*this, nRet);
			}
			return nRet;
		}

		void SetCurSelByData(LPARAM lParam) {
			ComboBox::SetCurSelByData(lParam);
			m_value = GetCurSelData();
		}

		void SetCurSelByWString(LPCWSTR wsz) {
			ComboBox::SetCurSelByWString(wsz);
			m_value = GetCurSelData();
		}

	protected:
		BOOL OnCommandControlFromParent(int nCode, int nID) override {
			if (nCode == CBN_SELCHANGE) {
				m_value = GetCurSelData();
			}

			return __super::OnCommandControlFromParent(nCode, nID);
		}

	protected:
		LPARAM m_value;
	};

	class ComboBoxWString : public ComboBox
	{
	public:
		ComboBoxWString() {}

		const ComboBoxWString& operator=(LPCWSTR wsz) {
			m_value = wsz; SetCurSelByWString(wsz);	return *this;
		}

		DWORD AttachToDlgItem(HWND hWnd, int nID) {
			RETURN_IF_ERROR(__super::AttachToDlgItem(hWnd, nID));
			SetFlag(WINDOW_FLAGS_MESSAGESFROMPARENT, TRUE);
			return ERROR_SUCCESS;
		}

		operator LPCWSTR() {
			return m_value.c_str();
		}

		int AddString(LPCWSTR sz) {
			int nRet = __super::AddString(sz);
			if (m_value == sz) {
				ComboBox_SetCurSel(*this, nRet);
			}
			return nRet;
		}

	protected:
		BOOL OnCommandControlFromParent(int nCode, int nID) override {
			if (nCode == CBN_SELCHANGE) {
				GetSelected(m_value);
			}

			return __super::OnCommandControlFromParent(nCode, nID);
		}

	protected:
		std::wstring m_value;
	};
}