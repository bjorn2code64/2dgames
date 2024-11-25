#pragma once

#include <menu.h>
#include <FileHandler.h>

class Doc : public FileHandler {
public:
	Doc() : m_pMenu(NULL), m_uMenuID(0), m_modified(false) {
	}

//	void New() {
//		FileHandler::Clear();
//		
//	}

	const Doc& operator=(LPCWSTR sz) { m_strFile = sz;	return *this; }

	void InitRFL(Menu* pMenu, UINT uMenuID, LPCWSTR sz) {
		m_pMenu = pMenu;
		m_uMenuID = uMenuID;
		m_rfl = sz;
		m_pMenu->SetMenuTitleByID(m_uMenuID, *sz ? sz : L"<recent file>");
		m_pMenu->EnableMenuItemByID(m_uMenuID, *sz);
	}

	LPCWSTR GetRFL() {
		return m_rfl.c_str();
	}

	void UpdateRFL() {
		m_rfl = *this;
		if (m_pMenu) {
			m_pMenu->SetMenuTitleByID(m_uMenuID, *this);
			m_pMenu->EnableMenuItemByID(m_uMenuID, *this[0]);
		}
	}

	void SetModifiedFlag(bool b = true) {
		m_modified = b;
	}

	bool IsModified() {
		return m_modified;
	}
/*
	bool OnLoaded() {
		UpdateRFL();
		SetModifiedFlag(false);
		return true;
	}

	bool OnSaved() {
		UpdateRFL();
		SetModifiedFlag(false);
		return true;
	}
*/

	// helpers
	void OnNew() {
		Clear();
		SetModifiedFlag(false);
	}

	void OnOpen(LPCWSTR sz) {
		*this = sz;
		SetModifiedFlag(false);
		UpdateRFL();
	}

	void OnSaveAs(LPCWSTR sz) {
		*this = sz;
		SetModifiedFlag(false);
		UpdateRFL();
	}
protected:
	std::wstring m_rfl;
	Menu* m_pMenu;
	UINT m_uMenuID;
	bool m_modified;
};