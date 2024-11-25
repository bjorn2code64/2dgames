#include "Dialog.h"

static INT_PTR CALLBACK l_DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Dialog* pDlg;
	if (message == WM_INITDIALOG)	// On creation, we get this (almost) first and the CGWindow* passed in the CreateParams (We miss a WM_SETFONT)
	{
		pDlg = (Dialog*)lParam;
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pDlg);
	}
	else
		pDlg = (Dialog*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (!pDlg)		return 0;

	return pDlg->DlgProc(hWnd, message, wParam, lParam);
};

DWORD Dialog::DoModal(int nDialogID, INT_PTR* pRet)
{
	Window* parent = GetParent();
	HWND hWndParent = parent ? HWND(*parent) : NULL;
	m_bDialogCreate = TRUE;
	*pRet = DialogBoxParam(Window::GetInstance(), MAKEINTRESOURCE(nDialogID), hWndParent, l_DlgProc, (LPARAM)this);
	if (*pRet <= 0) {
		return GetLastError();
	}

	return ERROR_SUCCESS;
}

DWORD Dialog::DoModal(LPCWSTR szTemplate, INT_PTR* pRet)
{
	Window* parent = GetParent();
	HWND hWndParent = parent ? HWND(*parent) : NULL;
	m_bDialogCreate = TRUE;
	*pRet = DialogBoxIndirectParam(Window::GetInstance(), (LPCDLGTEMPLATE)szTemplate, hWndParent, l_DlgProc, (LPARAM)this);
	if (*pRet <= 0) {
		return GetLastError();
	}

	return ERROR_SUCCESS;
}

BOOL DialogTemplate::Create(const w32Rect& r, LPCWSTR szTitle)
{
	HDC hdc = GetDC(NULL);
	if (!hdc)	return FALSE;

	NONCLIENTMETRICSW ncm = { sizeof(ncm) };
	if (!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0))
	{
		ReleaseDC(NULL, hdc);
		return FALSE;
	}

	// DLGTEMPLATEEX
	Add((WORD)1);
	Add((WORD)0xffff);
	Add((DWORD)0);
	Add((DWORD)0);
	Add((DWORD)(WS_CAPTION | WS_SYSMENU | DS_SETFONT | DS_MODALFRAME));
	m_pCount = (WORD*)m_ga.ContinuousAlloc((size_t)sizeof(WORD));
	*m_pCount = 0;
	Add((WORD)r.left);
	Add((WORD)r.top);
	Add((WORD)r.Width());
	Add((WORD)r.Height());
	Add(L"");
	Add(L"");
	Add(szTitle);
	if (ncm.lfMessageFont.lfHeight < 0)
	{
		ncm.lfMessageFont.lfHeight = -MulDiv(ncm.lfMessageFont.lfHeight,
			72, GetDeviceCaps(hdc, LOGPIXELSY));
	}

	Add((WORD)ncm.lfMessageFont.lfHeight); // point
	Add((WORD)ncm.lfMessageFont.lfWeight); // weight
	Add((BYTE)ncm.lfMessageFont.lfItalic); // Italic
	Add((BYTE)ncm.lfMessageFont.lfCharSet); // CharSet
	Add(ncm.lfMessageFont.lfFaceName);
	ReleaseDC(NULL, hdc);
	return TRUE;
}

void DialogTemplate::AddButton(const w32Rect& r, LPCWSTR sz, DWORD dwID, BOOL bDefault)
{
	size_t nAlign = m_ga.GetContinuousSize(FALSE) % 4;
	if (nAlign)
		m_ga.ContinuousAlloc(4 - nAlign);

	//DLGITEMTEMPLATEEX 
	Add((DWORD)0); // help id
	Add((DWORD)0); // window extended style
	Add((DWORD)(WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | (bDefault ? BS_DEFPUSHBUTTON : 0))); // style
	Add((WORD)r.left); // x
	Add((WORD)r.top); // y
	Add((WORD)r.Width()); // width
	Add((WORD)r.Height()); // height
	Add((DWORD)dwID); // control ID
	Add((DWORD)0x0080FFFF); // button
	Add(sz);
	Add((WORD)0); // no extra data

	(*m_pCount)++;
}

void DialogTemplate::AddStatic(const w32Rect& r, LPCWSTR sz, DWORD nID)
{
	size_t nAlign = m_ga.GetContinuousSize(FALSE) % 4;
	if (nAlign)
		m_ga.ContinuousAlloc(4 - nAlign);

	//DLGITEMTEMPLATEEX 
	Add((DWORD)0); // help id
	Add((DWORD)0); // window extended style
	Add((DWORD)(WS_CHILD | WS_VISIBLE)); // style
	Add((WORD)r.left); // x
	Add((WORD)r.top); // y
	Add((WORD)r.Width()); // width
	Add((WORD)r.Height()); // height
	Add((DWORD)nID); // control ID
	Add((DWORD)0x0082FFFF); // static
	Add(sz);
	Add((WORD)0); // no extra data

	(*m_pCount)++;
}

void DialogTemplate::AddEdit(const w32Rect& r, LPCWSTR sz, DWORD dwID)
{
	size_t nAlign = m_ga.GetContinuousSize(FALSE) % 4;
	if (nAlign)
		m_ga.ContinuousAlloc(4 - nAlign);

	//DLGITEMTEMPLATEEX 
	Add((DWORD)0); // help id
	Add((DWORD)0); // window extended style
	Add((DWORD)(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL)); // style
	Add((WORD)r.left); // x
	Add((WORD)r.top); // y
	Add((WORD)r.Width()); // width
	Add((WORD)r.Height()); // height
	Add((DWORD)dwID); // control ID
	Add((DWORD)0x0081FFFF); // edit
	Add(sz);
	Add((WORD)0); // no extra data

	(*m_pCount)++;
}

