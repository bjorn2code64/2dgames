#pragma once

#include <Dialog.h>
#include "WLSyslink.h"

class DialogAbout : public DialogTemplate
{
public:
	DialogAbout(Window* pParent, LPCWSTR szAppName, LPCWSTR szURL, int nIconID) :
		DialogTemplate(pParent), m_szURL(szURL), m_nIconID(nIconID)
	{
		Create(w32Rect(0, 0, 200, 40), L"About");
		AddStatic(w32Rect(40, 10, w32Size(100, 10)), szAppName);
		AddButton(w32Rect(145, 8, w32Size(50, 12)), L"OK", IDOK, TRUE);
	}

	DWORD OnInitDialog(HWND hWnd)
	{
		w32Rect r;
		::SetRect(r, 11, 11, 29, 29);
		::MapDialogRect(hWnd, r);

		m_icon.CreateAndShow(this, m_nIconID, SW_SHOW);
		::SetWindowPos(m_icon, NULL, r.left, r.top, r.Width(), r.Height(), SWP_NOZORDER);

		RETURN_IF_ERROR(m_syslink.CreateAndShow(this, m_szURL));
		::SetRect(r, 40, 20, 140, 30);
		::MapDialogRect(hWnd, r);
		::SetWindowPos(m_syslink, NULL, r.left, r.top, r.Width(), r.Height(), SWP_NOZORDER);

		return ERROR_SUCCESS;
	}

protected:
	WLSyslink m_syslink;
	LPCWSTR m_szURL;
	CommonControls::Static m_icon;
	int m_nIconID;
};