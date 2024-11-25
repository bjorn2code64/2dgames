#pragma once

#include "Window.h"
#include "GAlloc.h"
#include "CommonControl.h"
#include "CommDlg.h"

class Dialog : public Window
{
public:
	// Must have either a parent or an INST if main window
	Dialog(Window* parent, int nID = 0, WORD wFlags = 0) : Window(0, parent), m_nID(nID), m_bDialogCreate(FALSE) {
		SetFlag(wFlags, TRUE);
	}
	Dialog(int nID, WORD wFlags = 0) : m_nID(nID), m_bDialogCreate(FALSE) {
		SetFlag(wFlags, TRUE);
	}
	Dialog(WORD wFlags = 0) : m_bDialogCreate(FALSE), m_nID(0) {
		SetFlag(wFlags, TRUE);
	}

	virtual ~Dialog() {}

	DWORD DoModal(int nDialogID, INT_PTR* pRet);
	DWORD DoModal(INT_PTR* pRet) { return DoModal(m_nID, pRet); }
//	BOOL Create(int nDialogID);
	DWORD DoModal(LPCWSTR szTemplate, INT_PTR* pRet);
	void CloseDialog(INT_PTR nID) {
		if (nID == IDOK)
			OnOK();

		if (nID == IDCANCEL)
			OnCancel();

		if (m_bDialogCreate)
			EndDialog(*this, nID);
		else
			DestroyWindow(*this);
	}

	HWND GetDlgItem(int nID) { return ::GetDlgItem(*this, nID); }

	// Callbacks
	virtual DWORD OnInitDialog(HWND /*hWnd*/) {	// Return ERROR_SUCCESS or error code
		return InitExts(this);
	}
	virtual UINT_PTR DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
//		LRESULT lRet = 0;
//		if (OnMessage(hWnd, message, wParam, lParam, &lRet))	// Give the derived window first crack at it
	//		return lRet;

		LRESULT l = WndProcExts(hWnd, uMsg, wParam, lParam);
		if (l)	return l;

		switch (uMsg)	// Dialog specific messages
		{
		case WM_INITDIALOG:
		{
			SetHWnd(hWnd);
			//		InitExts(this);

			Center(GetParent());	// Move dialog to center it in the middle of its parent

			DWORD dwRet = OnInitDialog(hWnd);
			if (dwRet != ERROR_SUCCESS)
				Window::ReportError(dwRet);
			return (INT_PTR)TRUE;
		}

		case WM_COMMAND:
		{
			// Check for OK/Cancel and terminate the dialog automatically
			if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
			{
				CloseDialog(LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
		}
		break;

		case WM_CTLCOLORSTATIC:	// readonly/disabled edits and statics sends to parent
		case WM_CTLCOLOREDIT:	// enabled/writable edits sends to parent
		{
			HWND hWndChild = HWND(lParam);
			if (hWndChild)
			{
				CommonControls::CommonControl* cc = (CommonControls::CommonControl*)::GetWindowLongPtr(hWndChild, GWLP_USERDATA);	// Send it to the child window
				if (cc)
					return (UINT_PTR)cc->OnColorCtrlFromParent((HDC)wParam);
			}
		}

		default:
			break;
		}

		return Window::CrackMessage(hWnd, uMsg, wParam, lParam);
	}

	virtual void OnOK() {}
	virtual void OnCancel() {}
	virtual HBRUSH OnColorCtrl(HWND, HDC) { return NULL; }

protected:
	virtual void OnClose() { OnCancel();  CloseDialog(IDCANCEL); }	// User hits the close button

protected:
	int m_nID;
	BOOL m_bDialogCreate;	// was this dialog created by Dialog...() and not CreateDialog()?
};

class DialogTemplate : public Dialog
{
public:
	DialogTemplate(Window* pParent) : Dialog(pParent), m_pCount(NULL) {}
	DialogTemplate() : m_pCount(NULL) {}

	BOOL Create(const w32Rect& r, LPCWSTR szTitle);
	void AddButton(const w32Rect& r, LPCWSTR sz, DWORD dwID, BOOL bDefault = FALSE);
	void AddStatic(const w32Rect& r, LPCWSTR sz, DWORD nID = (DWORD)-1);
	void AddEdit(const w32Rect& r, LPCWSTR sz, DWORD dwID);

	DWORD DoModal(INT_PTR* pRet)
	{
		LPTSTR sz = (LPTSTR)m_ga.AllocACopy();
		DWORD dwRet = Dialog::DoModal(sz, pRet);
		delete[] sz;
		return dwRet;
	}

protected:
	void Add(BYTE b) { *(BYTE*)(m_ga.ContinuousAlloc((size_t)sizeof(BYTE))) = b; }
	void Add(WORD w) { *(WORD*)(m_ga.ContinuousAlloc((size_t)sizeof(WORD))) = w; }
	void Add(DWORD dw) { *(DWORD*)(m_ga.ContinuousAlloc((size_t)sizeof(DWORD))) = dw; }
	void Add(LPCWSTR sz)
	{
		size_t nLen = wcslen(sz) + 1;
		void* p = m_ga.ContinuousAlloc(nLen * 2);
		memcpy(p, sz, nLen * 2);
	}

protected:
	WORD* m_pCount;
	CGAlloc m_ga;
};

class DialogSimple : public DialogTemplate
{
public:
	int m_Width = 200;
	int m_Height = 47;
	int m_Border = 7;
	int m_ButtonWidth = 50;
	int m_LineHeight = 12;
	int m_midPointX = 80;
	bool m_cancel = true;
	bool m_ok = true;

public:
	DialogSimple(Window* pParent) :
		DialogTemplate(pParent)
	{}

	DWORD DoModal(LPCTSTR szTitle, LPCTSTR szRequest, INT_PTR* ret)
	{
		Create(w32Rect(0, 0, m_Width, m_Height), szTitle);

		// First Line
		int y = m_Border;
		AddStatic(w32Rect(m_Border, y, m_midPointX, y + m_LineHeight), szRequest);

		// 2nd Line
		y += m_LineHeight + m_Border;
		int x = m_Width - m_Border - m_ButtonWidth;
		if (m_cancel) {
			AddButton(
				w32Rect(x, y, x + m_ButtonWidth, y + m_LineHeight),
				L"Cancel",
				IDCANCEL);
			x -= (m_Border + m_ButtonWidth);
		}

		if (m_ok) {
			AddButton(
				w32Rect(x, y, x + m_ButtonWidth, y + m_LineHeight),
				L"OK",
				IDOK,
				TRUE);
			x -= (m_Border + m_ButtonWidth);
		}

		return DialogTemplate::DoModal(ret);
	}

	virtual void AddCustom() {}

protected:
	virtual void InitControl(const w32Rect& r) = 0;

	DWORD OnInitDialog(HWND hWnd) override
	{
		w32Rect r;
		::SetRect(&r, m_midPointX + m_Border, m_Border, m_Width - m_Border, m_Border + m_LineHeight);
		::MapDialogRect(hWnd, &r);

		InitControl(r);

		m_buttonOK.AttachToDlgItem(hWnd, IDOK);

		return ERROR_SUCCESS;
	}

	void UpdateOKButton(bool b) {
		::EnableWindow(m_buttonOK, b);
	}

	CommonControls::Button m_buttonOK;
};

class DialogGetText : public DialogSimple
{
public:
	DialogGetText(Window* pParent, int limitText) :
		DialogSimple(pParent),
		m_limitText(limitText),
		m_edit([this](LPCWSTR sz) {
			bool bOK = (*sz && (std::find(this->m_invalids.begin(), this->m_invalids.end(), sz) == this->m_invalids.end()));
			this->UpdateControls();
			return bOK;
		})
	{}

protected:
	void InitControl(const w32Rect& r) {
		m_edit.CreateAndShow(this, SW_SHOW);
		m_edit.AddWindowStyles(WS_TABSTOP);
		m_edit.LimitText(m_limitText);
		::SetWindowPos(m_edit, NULL, r.left, r.top, r.Width(), r.Height(), 0);
		Edit_SetSel(m_edit, 0, -1);
		::SetFocus(m_edit);
	}

	void UpdateControls() {
		UpdateOKButton(m_edit.IsValid());
	}

protected:
	int m_limitText;

public:
	CommonControls::EditValidator m_edit;
	std::vector<std::wstring> m_invalids;
};

class DialogCombo : public DialogSimple
{
public:
	DialogCombo(Window* pParent) : DialogSimple(pParent), m_selectedIndex(-1), m_selectedData(-1)
	{}

protected:
	void InitControl(const w32Rect& r) {
		w32Rect rExpanded(r);
		rExpanded.bottom += rExpanded.Height() * 5;
		m_combo.CreateAndShow(this, CBS_DROPDOWNLIST);
		m_combo.SetRect(rExpanded);
		for (auto op : m_options) {
			int index = m_combo.AddString(op.first.c_str(), op.second);
			if ((op.first == m_selected) || (op.second == m_selectedData)) {
				ComboBox_SetCurSel(m_combo, index);
			}
		}

		// Set it to the first option if none have been selected
		if ((ComboBox_GetCurSel(m_combo) == -1) && (ComboBox_GetCount(m_combo) > 0))
			ComboBox_SetCurSel(m_combo, 0);

		::SetFocus(m_combo);
	}

	void OnOK() override {
		m_combo.GetSelected(m_selected);
		m_selectedIndex = ComboBox_GetCurSel(m_combo);
		m_selectedData = m_combo.GetCurSelData();
	}

public:
	CommonControls::ComboBox m_combo;

public:
	std::vector<std::pair<std::wstring, LPARAM>> m_options;	// in only
	std::wstring m_selected;				// in/out
	int m_selectedIndex;					// out only
	LPARAM m_selectedData;					// out only
};

// TODO: Should this use DialogSimple?
class DialogGetInt : public DialogTemplate
{
public:
	DialogGetInt(Window* pParent, int nMin = 0, int nMax = -1) :
		DialogTemplate(pParent),
		m_editInt(nMin, nMax)
	{}

	DWORD DoModal(LPCTSTR szTitle, LPCTSTR szRequest, INT_PTR* ret)
	{
		Create(w32Rect(0, 0, 195, 47), szTitle);
		AddStatic(w32Rect(7, 7, 80, 19), szRequest);
		AddButton(w32Rect(84, 26, 134, 38), L"OK", IDOK, TRUE);
		AddButton(w32Rect(138, 26, 188, 38), L"Cancel", IDCANCEL);
		return DialogTemplate::DoModal(ret);
	}

	DWORD OnInitDialog(HWND hWnd) override
	{
		w32Rect r;
		::SetRect(&r, 93, 7, 93 + 95, 7 + 12);
		::MapDialogRect(hWnd, &r);

		m_buttonOK.AttachToDlgItem(hWnd, IDOK);

		m_editInt.CreateAndShow(this, SW_SHOW);
		m_editInt.AddWindowStyles(WS_TABSTOP);
		::SetWindowPos(m_editInt, NULL, r.left, r.top, r.Width(), r.Height(), 0);
		Edit_SetSel(m_editInt, 0, -1);
		::SetFocus(m_editInt);

		return ERROR_SUCCESS;
	}

	CommonControls::EditInt m_editInt;
	CommonControls::Button m_buttonOK;
};

// TODO: Should this use DialogSimple?
class DialogGetDouble : public DialogTemplate
{
public:
	DialogGetDouble(Window* pParent) :
		DialogTemplate(pParent)
	{}

	DWORD DoModal(LPCTSTR szTitle, LPCTSTR szRequest, INT_PTR* ret)
	{
		Create(w32Rect(0, 0, 195, 47), szTitle);
		AddStatic(w32Rect(7, 7, 80, 19), szRequest);
		AddButton(w32Rect(84, 26, 134, 38), L"OK", IDOK, TRUE);
		AddButton(w32Rect(138, 26, 188, 38), L"Cancel", IDCANCEL);
		return DialogTemplate::DoModal(ret);
	}

	DWORD OnInitDialog(HWND hWnd) override
	{
		w32Rect r;
		::SetRect(&r, 93, 7, 93 + 95, 7 + 12);
		::MapDialogRect(hWnd, &r);

		m_buttonOK.AttachToDlgItem(hWnd, IDOK);

		m_edit.CreateAndShow(this, SW_SHOW);
		m_edit.AddWindowStyles(WS_TABSTOP);
		::SetWindowPos(m_edit, NULL, r.left, r.top, r.Width(), r.Height(), 0);
		Edit_SetSel(m_edit, 0, -1);
		::SetFocus(m_edit);

		return ERROR_SUCCESS;
	}

	CommonControls::EditDouble m_edit;
	CommonControls::Button m_buttonOK;
};

// TODO: Should this use DialogSimple?
class DialogGetDataItem : public DialogTemplate
{
public:
	DialogGetDataItem(Window* pParent, DataItem::type type) :
		DialogTemplate(pParent),
		m_editDataItem(type)
	{}

	DWORD DoModal(LPCTSTR szTitle, LPCTSTR szRequest, INT_PTR* ret)
	{
		Create(w32Rect(0, 0, 195, 47), szTitle);
		AddStatic(w32Rect(7, 7, 80, 19), szRequest);
		AddButton(w32Rect(84, 26, 134, 38), L"OK", IDOK, TRUE);
		AddButton(w32Rect(138, 26, 188, 38), L"Cancel", IDCANCEL);
		return DialogTemplate::DoModal(ret);
	}

	DWORD OnInitDialog(HWND hWnd) override
	{
		w32Rect r;
		::SetRect(&r, 93, 7, 93 + 95, 7 + 12);
		::MapDialogRect(hWnd, &r);

		m_buttonOK.AttachToDlgItem(hWnd, IDOK);

		m_editDataItem.CreateAndShow(this, SW_SHOW);
		m_editDataItem.AddWindowStyles(WS_TABSTOP);
		::SetWindowPos(m_editDataItem, NULL, r.left, r.top, r.Width(), r.Height(), 0);
		Edit_SetSel(m_editDataItem, 0, -1);
		::SetFocus(m_editDataItem);

		return ERROR_SUCCESS;
	}

	virtual BOOL OnCommandControl(HWND hWnd, int nCode, int nID)
	{
		if (nCode == EN_CHANGE) {
			::EnableWindow(m_buttonOK, m_editDataItem.IsValid());
		}
		return FALSE;
	}

	CommonControls::EditDataItem m_editDataItem;
	CommonControls::Button m_buttonOK;
};

class DialogColour
{
public:
	DialogColour(HWND parent) : m_parent(parent), m_rgbCurrent(0) {}

	DWORD DoModal(INT_PTR* ret) {
		CHOOSECOLOR cc;                 // common dialog box structure 
		static COLORREF acrCustClr[16]; // array of custom colors 
		HBRUSH hbrush;                  // brush handle

		// Initialize CHOOSECOLOR 
		ZeroMemory(&cc, sizeof(cc));
		cc.lStructSize = sizeof(cc);
		cc.hwndOwner = m_parent;
		cc.lpCustColors = (LPDWORD)acrCustClr;
		cc.rgbResult = m_rgbCurrent;
		cc.Flags = CC_FULLOPEN | CC_RGBINIT;

		if (ChooseColor(&cc) == TRUE)
		{
			hbrush = CreateSolidBrush(cc.rgbResult);
			m_rgbCurrent = cc.rgbResult;
			*ret = IDOK;
		}
		else
			*ret = IDCANCEL;

		return ERROR_SUCCESS;
	}

protected:
	HWND m_parent;

public:
	DWORD m_rgbCurrent;        // initial color selection
};

class StaticColour : public CommonControls::Static {
public:
	StaticColour(NotifyTarget& nt) : m_nt(nt), m_hbr(NULL), m_colour(RGB(0, 0, 0)) {}
	~StaticColour() {
		if (m_hbr)
			DeleteObject(m_hbr);
	}

	DWORD AttachToDlgItem(HWND hWndDialog, int nDlgID)
	{
		RETURN_IF_ERROR(Static::AttachToDlgItem(hWndDialog, nDlgID));
		SetFlag(WINDOW_FLAGS_MESSAGESFROMPARENT, TRUE);
		return ERROR_SUCCESS;
	}

	COLORREF operator=(COLORREF colour) {
		if (m_hbr)
			DeleteObject(m_hbr);
		m_hbr = CreateSolidBrush(m_colour = colour);
		RedrawWindow();
		return m_colour;
	}

	operator COLORREF() {
		return m_colour;
	}

	BOOL OnCommandControlFromParent(int nCode, int nID) override {
		if (nCode == BN_CLICKED) {
			DialogColour dlg(*this);
			dlg.m_rgbCurrent = m_colour;
			INT_PTR ret;
			if ((dlg.DoModal(&ret) == ERROR_SUCCESS) && (ret == IDOK)) {
				*this = dlg.m_rgbCurrent;
				m_nt.NotifyPost(m_colour);
			}
			return TRUE;
		}
		return FALSE;
	}

protected:
	HBRUSH OnColorCtrlFromParent(HDC hDC) {
		return m_hbr;
	}

	NotifyTarget& m_nt;
	COLORREF m_colour;
	HBRUSH m_hbr;
};

