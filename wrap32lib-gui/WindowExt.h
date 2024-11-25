#pragma once

#include <vector>

#include "utils.h"

class Window;

class WindowExt
{
public:
	WindowExt() {}

	virtual DWORD Init(Window* pOwner) = 0;

	// Stuff that gets called from CGWindow - Not called WndProc so it doesn't clash if we derive from both WE and GW
	virtual LRESULT WndProc(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/) { return 0; }

	// Adjust ClientRect dimentions if this control obscures part of it (statusbar at the bottom, toolbar at the top, etc.)
	virtual void AdjustClientRect(RECT* p) {}
private:
};

class WindowExtHost
{
public:
	void AddExt(WindowExt* ext) { m_exts.push_back(ext); }

	// Utility function to allow all exts to automatically adjust the client rect - e.g. toolbar, statusbar.
	void AdjustClientRect(RECT* p) {
		for (auto& ext : m_exts)
			ext->AdjustClientRect(p);
	}

	DWORD InitExts(Window* w) {
		for (auto& ext : m_exts) {
			RETURN_IF_ERROR(ext->Init(w));
		}

		return ERROR_SUCCESS;
	}

	LRESULT WndProcExts(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		for (auto& ext : m_exts) {
			LRESULT l = ext->WndProc(hWnd, message, wParam, lParam);
			if (l)
				return l;
		}
		return 0;
	}

protected:
	std::vector<WindowExt*> m_exts;
};

class WaitCursorExt : public WindowExt
{
public:
	WaitCursorExt() : m_bBusy(FALSE)
	{
		m_hCursorBusy = LoadCursor(NULL, IDC_WAIT);
		m_hCursorDefault = LoadCursor(NULL, IDC_ARROW);
	}

	void SetBusy(BOOL b) { m_bBusy = b;	SetCursor(m_bBusy ? m_hCursorBusy : m_hCursorDefault); }

protected:
	DWORD Init(Window* pOwner) {
		return ERROR_SUCCESS;
	}

	LRESULT WndProc(HWND, UINT message, WPARAM, LPARAM) override
	{
		if ((message == WM_SETCURSOR) && m_bBusy)
		{
			SetCursor(m_hCursorBusy);
			return 1;
		}

		return 0;
	}

protected:
	HCURSOR m_hCursorBusy;
	HCURSOR m_hCursorDefault;
	BOOL m_bBusy;
};

class ScrollBarExt : public WindowExt
{
public:
	static const WORD FLAGS_DONT_CLEAR_ON_INVALIDATE = 0x01;	// Help stop flickering
	static const WORD FLAGS_DONT_USE_SCROLLWINDOW = 0x02;	// Paint whole window on every change, not just gap

	ScrollBarExt(int sb, WORD flags = 0) : 
		m_bar(sb),
		m_message((sb == SB_VERT) ? WM_VSCROLL : WM_HSCROLL),
		m_lineClick(0),
		m_pageClick(0),
		m_range(0),
		m_page(0),
		m_pOwner(NULL),
		m_flags(flags)
	{
	}

	void SetLineClick(int line) {
		m_lineClick = line;
	}

	void SetRange(int range) {
		m_range = range;
		Set();
	}

	void SetPage(int page) {
		m_page = page;
		Set();
	}

	void SetPageClick(int page) {
		m_pageClick = page;
	}

	int GetPos();
	void SetPos(int x);

protected:
	DWORD Init(Window* pOwner) {
		m_pOwner = pOwner;
		return ERROR_SUCCESS;
	}

	void Set();

	LRESULT WndProc(HWND hWnd, UINT message, WPARAM, LPARAM) override;
	void OnMouseWheel(int16_t distance, WORD vkeys);

protected:
	int m_bar;
	UINT m_message;
	int m_range;
	int m_page;	// range for scrollbar UI
	int m_pageClick;
	int m_lineClick;
	Window* m_pOwner;
	WORD m_flags;
};

class DoubleBufferExt: public WindowExt
{
public:
	DoubleBufferExt() : m_pOwner(NULL) {}

	DWORD Init(Window* pOwner) {
		m_pOwner = pOwner;
		return ERROR_SUCCESS;
	}

	LRESULT WndProc(HWND hWnd, UINT message, WPARAM /*wParam*/, LPARAM /*lParam*/) override;
private:
	Window* m_pOwner;
};

