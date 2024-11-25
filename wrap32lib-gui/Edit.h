#pragma once

#include <StringValidator.h>
#include <StringParser.h>
#include <DataItem.h>

#include "CommonControl.h"

#include <string>
#include <sstream>

namespace CommonControls {

	class Edit : public CommonControl
	{
	public:
		DWORD CreateAndShow(Window* parent, int nCmdShow = SW_SHOW) {
			RETURN_IF_ERROR(__super::Create(parent, WC_EDIT, NULL, WS_CHILD | WS_BORDER | ES_AUTOHSCROLL));

			::ShowWindow(*this, nCmdShow);
			return ERROR_SUCCESS;
		}

		DWORD CreateAndShowStyle(Window* parent, DWORD style, int nCmdShow = SW_SHOW) {
			RETURN_IF_ERROR(__super::Create(parent, WC_EDIT, NULL, style | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL));

			::ShowWindow(*this, nCmdShow);
			return ERROR_SUCCESS;
		}

		virtual operator std::wstring() {
			LPWSTR sz = AllocWindowText();
			std::wstring s = sz;
			delete[] sz;
			return s;
		}

		BOOL GetWindowTextAsInt(int* pRet, bool hex = false) {
			std::wstring v = operator std::wstring();
			LPCWSTR sz = v.c_str();

			StringValidator sv(sz);
			if (!sv.IsValid(StringValidator::type::type_int)) {
				return FALSE;
			}

			WStringParser sp(sz);
			return hex ? sp.GetHexInt(*pRet) : sp.GetInt(*pRet);
		}

		double GetWindowTextAsDouble() {
			std::wstring v = operator std::wstring();
			return _wtof(v.c_str());
		}

		void SetWindowTextAsInt(int n, bool hex = false) {
			wchar_t buff[12];
			_snwprintf_s(buff, 12, hex ? L"%X" : L"%d", n);
			*this = buff;
		}

		virtual void operator=(LPCWSTR sz) {
			::SetWindowText(*this, sz);
		}

		virtual void operator=(const std::wstring& w) {
			::SetWindowText(*this, w.c_str());
		}

		void LimitText(int n) {
			Edit_LimitText(*this, n);
		}

		static const int m_nDefaultHeight = 23;
	};

	class EditString : public Edit
	{
	public:
		void operator=(LPCWSTR sz) override {
			m_value = sz;
			__super::operator=(sz);
		}

		void operator=(const std::wstring& w) override {
			m_value = w;
			__super::operator=(w);
		}

		DWORD CreateAndShowStyle(Window* parent, DWORD style, int nCmdShow = SW_SHOW) {
			RETURN_IF_ERROR(__super::CreateAndShowStyle(parent, style, nCmdShow));

			SetFlag(WINDOW_FLAGS_MESSAGESFROMPARENT, TRUE);
			*this = m_value.c_str();
			::ShowWindow(*this, nCmdShow);
			return ERROR_SUCCESS;
		}

		DWORD CreateAndShow(Window* parent, int nCmdShow = SW_SHOW) {
			RETURN_IF_ERROR(__super::CreateAndShow(parent, nCmdShow));

			SetFlag(WINDOW_FLAGS_MESSAGESFROMPARENT, TRUE);
			*this = m_value.c_str();
			::ShowWindow(*this, nCmdShow);
			return ERROR_SUCCESS;
		}

		operator LPCWSTR() {
			return m_value.c_str();
		}

		DWORD AttachToDlgItem(HWND hWnd, int nID) {
			RETURN_IF_ERROR(__super::AttachToDlgItem(hWnd, nID));
			SetFlag(WINDOW_FLAGS_MESSAGESFROMPARENT, TRUE);
			*this = m_value.c_str();
			return ERROR_SUCCESS;
		}

	protected:
		BOOL OnCommandControlFromParent(int nCode, int nID) override {
			if (nCode == EN_CHANGE) {
				LPWSTR sz = AllocWindowText();
				m_value = sz;
				delete[] sz;
			}

			return __super::OnCommandControlFromParent(nCode, nID);
		}

	protected:
		std::wstring m_value;
	};

	class EditValidator : public EditString
	{
	public:
		typedef std::function<bool(LPCWSTR)> func;

		EditValidator(func funcIsValid = nullptr) : m_funcIsValid(funcIsValid) {
			m_bIsValid = (m_funcIsValid && m_funcIsValid(L""));
		}

		bool IsValid() { return m_bIsValid; }

		void operator=(LPCWSTR sz) override {
			__super::operator=(sz);
		}

		void operator=(const std::wstring& w) override {
			__super::operator=(w);
		}
		
		DWORD AttachToDlgItem(HWND hWnd, int nID) {
			SetFlag(WINDOW_FLAGS_MESSAGESFROMPARENT, TRUE);
			return __super::AttachToDlgItem(hWnd, nID);
		}

	protected:
		HBRUSH OnColorCtrlFromParent(HDC hDC) override {
			wchar_t buff[1024];
			::GetWindowText(*this, buff, 1024);
			m_bIsValid = (m_funcIsValid && m_funcIsValid(buff));
			if (m_bIsValid)
				return NULL;

			if (!::IsWindowEnabled(*this))
				return NULL;

			COLORREF crBk = RGB(255, 127, 127);

			SetBkColor(hDC, crBk); // Set to red
			SetDCBrushColor(hDC, crBk);
			return (HBRUSH)GetStockObject(DC_BRUSH); // return a DC brush.
		}

		BOOL OnCommandControlFromParent(int nCode, int nID) override {
			if (nCode == EN_CHANGE) {	// May be a colour change - force a redraw
				RedrawWindow();
			}

			return __super::OnCommandControlFromParent(nCode, nID);
		}

	protected:
		func m_funcIsValid;
		bool m_bIsValid;
	};

	class EditStringValidator : public EditValidator
	{
	public:
		EditStringValidator(LPCWSTR regex) : m_regex(regex), EditValidator(
			[this](LPCWSTR sz) {
				StringValidator sv(sz);
				return sv.IsValid(m_regex.c_str()) && (std::find(m_invalids.begin(), m_invalids.end(), sz) == m_invalids.end());
			}) 
		{}

		void AddInvalid(LPCWSTR sz) {			m_invalids.push_back(sz);		}
		void AddInvalid(const std::vector<std::wstring>& invalids) {
			for (auto& i : invalids)
				AddInvalid(i.c_str());
		}

		void operator=(LPCWSTR sz) override {
			EditString::operator=(sz);
		}

		BOOL CreateAndShow(Window* parent, int nCmdShow = SW_SHOW) {
			RETURN_IF_ERROR(__super::CreateAndShow(parent, nCmdShow));

			SetFlag(WINDOW_FLAGS_MESSAGESFROMPARENT, TRUE);
			::SetWindowText(*this, m_value.c_str());
			return ERROR_SUCCESS;
		}

		DWORD AttachToDlgItem(HWND hWnd, int nID) {
			RETURN_IF_ERROR(__super::AttachToDlgItem(hWnd, nID));
			SetFlag(WINDOW_FLAGS_MESSAGESFROMPARENT, TRUE);

			LimitText(253);	// max len possible
			::SetWindowText(*this, m_value.c_str());

			return ERROR_SUCCESS;
		}

		BOOL IsValid() {
			StringValidator sv(m_value.c_str());
			return sv.IsValid(m_regex.c_str()) && (std::find(m_invalids.begin(), m_invalids.end(), m_value.c_str()) == m_invalids.end());
		}

	protected:
		std::wstring m_regex;
		std::vector<std::wstring> m_invalids;
	};

	class EditInt : public EditValidator
	{
	public:
		EditInt(int nMin, int nMax, bool hex = false) : m_nMin(nMin), m_nMax(nMax), m_hex(hex), EditValidator(
			[this](LPCWSTR sz) {
				int n;
				return this->Parse(sz, n);
			}
		)
		{
		}

			BOOL CreateAndShow(Window* parent, int nCmdShow = SW_SHOW) {
				RETURN_IF_ERROR(__super::CreateAndShow(parent, nCmdShow));
				SetFlag(WINDOW_FLAGS_MESSAGESFROMPARENT, TRUE);
				::SetWindowText(*this, m_value.c_str());
				return ERROR_SUCCESS;
			}

			const EditInt& operator=(int n) {
				if (m_hex) {
					std::wostringstream oss;  // note the 'w'
					oss << std::hex << n;
					m_value = oss.str();
				}
				else
					m_value = std::to_wstring(n);

				::SetWindowText(*this, m_value.c_str());
				return *this;
			}

			DWORD AttachToDlgItem(HWND hWnd, int nID) {
				RETURN_IF_ERROR(__super::AttachToDlgItem(hWnd, nID));
				SetFlag(WINDOW_FLAGS_MESSAGESFROMPARENT, TRUE);

				LimitText(11);	// max int possible
				::SetWindowText(*this, m_value.c_str());
				return ERROR_SUCCESS;
			}

			BOOL IsValid() {
				int n;
				GetWindowTextAsInt(&n, m_hex);
				return ((n >= m_nMin) && (n <= m_nMax));
			}

			void SetLimits(int nMin, int nMax) {
				m_nMin = nMin;	m_nMax = nMax;	RedrawWindow();
			}

			bool Parse(LPCWSTR sz, int& n) {
				StringValidator sv(sz);
				if (!sv.IsValid(this->m_hex ? StringValidator::type::type_hexint : StringValidator::type::type_int))
					return false;

				WStringParser sp(sz);
				if (!(this->m_hex ? sp.GetHexInt(n) : sp.GetInt(n)))
					return false;
				return ((n >= this->m_nMin) && (n <= this->m_nMax));
			}

			operator int() {
				int n;
				if (!Parse(m_value.c_str(), n))
					return 0;

				return n;
			}

	protected:
		int m_nMin;
		int m_nMax;
		bool m_hex;
	};

	class EditHostname : public EditValidator
	{
	public:
		EditHostname() : EditValidator(
			[this](LPCWSTR sz) {
				StringValidator sv(sz);
				return sv.IsValid(StringValidator::type::type_hostname);
			}) {}

			void operator=(LPCWSTR sz) override {
				EditString::operator=(sz);
			}

			BOOL CreateAndShow(Window* parent, int nCmdShow = SW_SHOW) {
				RETURN_IF_ERROR(__super::CreateAndShow(parent, nCmdShow));

				SetFlag(WINDOW_FLAGS_MESSAGESFROMPARENT, TRUE);
				::SetWindowText(*this, m_value.c_str());
				return ERROR_SUCCESS;
			}

			DWORD AttachToDlgItem(HWND hWnd, int nID) {
				RETURN_IF_ERROR(__super::AttachToDlgItem(hWnd, nID));
				SetFlag(WINDOW_FLAGS_MESSAGESFROMPARENT, TRUE);

				LimitText(253);	// max len possible
				::SetWindowText(*this, m_value.c_str());

				return ERROR_SUCCESS;
			}

			BOOL IsValid() {
				StringValidator sv(m_value.c_str());
				return sv.IsValid(StringValidator::type::type_hostname);
			}
	};

	class EditDouble : public EditValidator
	{
	public:
		EditDouble() : EditValidator(
			[this](LPCWSTR sz) {
				StringValidator sv(sz);
				return sv.IsValid(StringValidator::type::type_double);
			})
		{
		}

			BOOL CreateAndShow(Window* parent, int nCmdShow = SW_SHOW) {
				RETURN_IF_ERROR(__super::CreateAndShow(parent, nCmdShow));
				SetFlag(WINDOW_FLAGS_MESSAGESFROMPARENT, TRUE);
				::SetWindowText(*this, m_value.c_str());
				return ERROR_SUCCESS;
			}

			const EditDouble& operator=(double d) {
				m_value = std::to_wstring(d);
				::SetWindowText(*this, m_value.c_str());
				return *this;
			}

			DWORD AttachToDlgItem(HWND hWnd, int nID) {
				RETURN_IF_ERROR(__super::AttachToDlgItem(hWnd, nID));
				SetFlag(WINDOW_FLAGS_MESSAGESFROMPARENT, TRUE);

				LimitText(32);
				::SetWindowText(*this, m_value.c_str());

				return ERROR_SUCCESS;
			}

			operator double() {
				return _wtof(m_value.c_str());
			}
	};

	class EditDataItem : public EditValidator
	{
	public:
		EditDataItem(DataItem::type type) : m_type(type),
			EditValidator([this](LPCWSTR sz) {	DataItem di(m_type); return this->Parse(sz, di); })
		{}

		BOOL CreateAndShow(Window* parent, int nCmdShow = SW_SHOW) {
			RETURN_IF_ERROR(__super::CreateAndShow(parent, nCmdShow));
			SetFlag(WINDOW_FLAGS_MESSAGESFROMPARENT, TRUE);
			::SetWindowText(*this, m_value.c_str());
			return ERROR_SUCCESS;
		}

		const EditDataItem& operator=(const DataItem& di) {
			di.GetAsString(m_value);

			::SetWindowText(*this, m_value.c_str());
			return *this;
		}

		DWORD AttachToDlgItem(HWND hWnd, int nID) {
			RETURN_IF_ERROR(__super::AttachToDlgItem(hWnd, nID));
			SetFlag(WINDOW_FLAGS_MESSAGESFROMPARENT, TRUE);

			LimitText(11);	// max int possible
			::SetWindowText(*this, m_value.c_str());
			return ERROR_SUCCESS;
		}

		BOOL IsValid() {
			wchar_t buff[256];
			::GetWindowText(*this, buff, 256);
			DataItem di(m_type);
			return Parse(buff, di);
		}

		bool Parse(LPCWSTR sz, DataItem& di) {
			if (!di.SetFromString(sz))
				return false;
			return true;

			//				return ((di >= m_min) && (di <= m_max));
		}

		operator const DataItem () {
			DataItem di(m_type);
			di.SetFromString(m_value.c_str());
			return di;
		}

	protected:
		DataItem::type m_type;
	};
}
