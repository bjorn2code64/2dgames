#pragma once

#include "Window.h"

class Menu
{
public:
	Menu(HMENU h = NULL) : m_hMenu(h), m_bManaged(FALSE) {}
	~Menu() { CleanUp(); }

	operator HMENU() {	return m_hMenu;	}

	HMENU operator=(HMENU hMenu) { m_hMenu = hMenu; m_bManaged = FALSE;  return m_hMenu; }

	void Create() {
		CleanUp();
		m_hMenu = ::CreateMenu();
		m_bManaged = TRUE;
	}

	void CreatePopup() {
		CleanUp();
		m_hMenu = ::CreatePopupMenu();
		m_bManaged = TRUE;
	}

	BOOL Attach(Menu& menu) {
		CleanUp();
		m_hMenu = GetSubMenu(*this, 0);
		return IsValid();
	}

	void Detach() { CleanUp(); }

	DWORD Load(int nResource) {
		CleanUp();
		m_hMenu = ::LoadMenu(Window::GetInstance(), MAKEINTRESOURCE(nResource));
		if (m_hMenu) {
			m_bManaged = true;
			return ERROR_SUCCESS;
		}

		return GetLastError();
	}

	BOOL IsValid() { return m_hMenu != NULL; }

	// Set the menu to the owner
	DWORD SetMenu(HWND hWndOwner) { return ::SetMenu(hWndOwner, m_hMenu) ? ERROR_SUCCESS : GetLastError(); }
	DWORD HideMenu(HWND hWndOwner) { return ::SetMenu(hWndOwner, nullptr) ? ERROR_SUCCESS : GetLastError(); }

	void SetCheckByID(UINT uID, BOOL bChecked) {
		::CheckMenuItem(m_hMenu, uID, MF_BYCOMMAND | (bChecked ? MF_CHECKED : MF_UNCHECKED));
	}

	BOOL GetCheckByID(UINT uID) {
		return ::GetMenuState(m_hMenu, uID, MF_BYCOMMAND) & MF_CHECKED;
	}

	void EnableMenuItemByID(UINT uID, BOOL bEnabled) {
		::EnableMenuItem(m_hMenu, uID, MF_BYCOMMAND | (bEnabled ? MF_ENABLED : MF_GRAYED));
	}

	void DeleteMenuItemByID(UINT uID) {
		::DeleteMenu(m_hMenu, uID, MF_BYCOMMAND);
	}

	DWORD InsertMenuItem(int index, UINT id, LPCWSTR title) {
		MENUITEMINFO mii;
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_ID | MIIM_STRING;
		mii.fType = MFT_STRING;
		mii.wID = id;
		mii.dwTypeData = (LPWSTR)title;

		if (!::InsertMenuItem(m_hMenu, index, TRUE, &mii))
			return ERROR_SUCCESS;
		return GetLastError();
	}

	void SetSubMenu(UINT indexOrID, HMENU hSubMenu, BOOL byIndex) {
		MENUITEMINFO info;
		info.cbSize = sizeof(MENUITEMINFO);
		info.fMask = MIIM_SUBMENU;
		info.hSubMenu = hSubMenu;
		::SetMenuItemInfo(*this, indexOrID, byIndex, &info);
	}


	void AddSubMenu(UINT id, int index, LPCWSTR title) {
		GetMenuItemCount(m_hMenu);
		HMENU hMenu = GetSubMenu(m_hMenu, index);

		MENUITEMINFO mii;
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_STRING | MIIM_ID;
		mii.dwTypeData = (LPWSTR)title;
		mii.cch = (UINT)wcslen(title);
		mii.wID = id;

		::InsertMenuItem(hMenu, GetMenuItemCount(hMenu), TRUE, &mii);
	}

	std::wstring GetTitleStringById(UINT id) {
		MENUITEMINFO mii;
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_STRING;
		mii.dwTypeData = NULL;
		::GetMenuItemInfo(m_hMenu, id, FALSE, &mii);

		UINT size = mii.cch + 1;
		wchar_t* buff = new wchar_t[size];

		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_STRING;
		mii.dwTypeData = buff;
		mii.cch = size * sizeof(wchar_t);
		::GetMenuItemInfo(m_hMenu, id, FALSE, &mii);

		std::wstring ret = buff;
		delete[] buff;
		return ret;
	}

	void SetMenuTitleByID(UINT uID, LPCTSTR sz)
	{
		MENUITEMINFO mii;
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_TYPE;
		mii.fType = MFT_STRING;
		mii.dwTypeData = (LPTSTR)sz;
		mii.cch = lstrlen(sz);
		::SetMenuItemInfo(m_hMenu, uID, FALSE, &mii);
	}

	void TrackPopupMenu(POINT ptScreen, HWND hWndNotify) {
		::TrackPopupMenu(m_hMenu, TPM_TOPALIGN | TPM_LEFTALIGN, ptScreen.x, ptScreen.y, 0, hWndNotify, NULL);
	}

	void TrackPopupSubMenu(POINT ptScreen, HWND hWndNotify) {
		::TrackPopupMenu(::GetSubMenu(*this, 0), TPM_TOPALIGN | TPM_LEFTALIGN, ptScreen.x, ptScreen.y, 0, hWndNotify, NULL);
	}
protected:
	void CleanUp()
	{
		if (m_hMenu && m_bManaged)
			::DestroyMenu(m_hMenu);
		m_hMenu = NULL;
		m_bManaged = FALSE;
	}

protected:
	HMENU m_hMenu;
	BOOL m_bManaged;	// Did we create, therefore do we need to destroy?
};


class MenuExt : public WindowExt, public Menu
{
public:
	MenuExt(int nResource) : m_nResource(nResource), m_hOwner(nullptr) {
	}

	DWORD Init(Window* pOwner) override { m_hOwner = *pOwner; return Load(m_nResource); }
	DWORD ShowMenu(bool bShow) {
		return bShow ? SetMenu(m_hOwner) : HideMenu(m_hOwner);
	}

	DWORD Load(int nResource) {
		RETURN_IF_ERROR(Menu::Load(nResource));
		return SetMenu(m_hOwner);
	}

protected:
	int m_nResource;
	HWND m_hOwner;
};

class PopupMenuExt : public MenuExt
{
public:
	PopupMenuExt(int nResource) : MenuExt(nResource), m_pWinNotify(NULL), m_enabled(true) {}
	~PopupMenuExt() {}

	DWORD Init(Window* pOwner) override { m_hOwner = *pOwner;  return Menu::Load(m_nResource); }	//don't call the base class

	void SetNotifyWindow(Window* pNotify) { m_pWinNotify = pNotify;  }

	void OnContextMenu(const w32Point& pt)
	{
		if (!m_enabled)
			return;

		m_ptClick = pt;
		::ScreenToClient(m_hOwner, &m_ptClick);
		SetForegroundWindow(m_hOwner);	// ensure menu closes properly, especially if popped up in tray
		TrackPopupSubMenu(0, pt);
	}

	w32Point GetClickPos() { 
		return m_ptClick; 
	}

	void TrackPopupSubMenu(int nSubMenu, POINT pt)
	{
		HMENU hMenu = ::GetSubMenu(m_hMenu, nSubMenu);
		::TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, m_pWinNotify ? *m_pWinNotify : m_hOwner, NULL);
	}

	void Enable(bool b) { m_enabled = b;  }

protected:
	LRESULT WndProc(HWND, UINT message, WPARAM, LPARAM lParam) override
	{
		if (message == WM_CONTEXTMENU) {
			OnContextMenu(lParam);
			return 1;
		}

		return 0;
	}

	Window* m_pWinNotify;
	w32Point m_ptClick;
	bool m_enabled;
};

