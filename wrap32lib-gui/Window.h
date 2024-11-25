#pragma once

#include <NotifyTarget.h> 
#include "utils.h"
#include "WindowExt.h"
#include <functional>
#include <AppMessage.h>

#include <string>

#define WINDOW_FLAGS_DONTREGISTER		0x0001	// Don't register this window (for common controls)
#define WINDOW_FLAGS_CRACKPAINT			0x0002	// Call the OnPaint() virtual on a WM_PAINT message
#define WINDOW_FLAGS_MESSAGESFROMPARENT	0x0004	// Pass messages from parent back down to control itself
#define WINDOW_FLAGS_NOTIFYCOMMANDS		0x0008	// Divert old style notify messages (button, combobox, ...) from OnCommandControl() to OnNotify()
#define WINDOW_FLAGS_ADJUSTSIZE			0x0010	// Adjust size before passing to OnSize()
#define WINDOW_FLAGS_CRACKCREATE		0x0020	// Call the onCreate() virtual on a WM_CREATE message
#define WINDOW_FLAGS_CRACKTIMER			0x0040	// Call OnTimer() on a WM_TIMER message
#define WINDOW_FLAGS_QUITONCLOSE		0x0080	// Post Quit Message if this window is closed

class Window : public WindowExtHost
{
public:
	static void LibInit(HINSTANCE h) { m_hInstance = h; }

	Window(WORD wFlags = 0, Window* parent = NULL) : m_hWnd(NULL), m_wFlags(wFlags), m_parent(parent) {}

	// Create() - Return Error Code
	DWORD Create(Window* parent, LPCWSTR lpszClassName, LPCWSTR lpszWindowName, DWORD dwStyles, std::function<void(WNDCLASSEX&)> fRegClassOpts = nullptr, int nID = 0, DWORD dwExStyles = 0);

	DWORD CreateOverlapped(LPCWSTR lpszClassAndTitleName, std::function<void(WNDCLASSEX&)> fRegClassOpts = nullptr) {
		return Create(NULL, lpszClassAndTitleName, lpszClassAndTitleName, WS_OVERLAPPEDWINDOW, fRegClassOpts);
	}

	BOOL Show(int nCmdShow = SW_SHOW) {
		return ::ShowWindow(m_hWnd, nCmdShow);	// Returns if window was previously visible
	}

	operator HWND() const {	return m_hWnd;	}

	virtual BOOL OnCommandMenu(int nID) { return FALSE;	 }	// Return TRUE if we handled it
	virtual BOOL OnCommandAccel(int nID) { return FALSE; }	// Return TRUE if we handled it
	virtual BOOL OnCommandControl(HWND hWnd, int nCode, int nID) { return FALSE; }	// Return TRUE if we handled it
	virtual BOOL OnCommandControlFromParent(int nCode, int nID) { return FALSE; }	// Return TRUE if we handled it
	virtual BOOL OnNotify(HWND /*hWnd*/, UINT /*nCode*/, LPNMHDR /*lpNMHDR*/) { return FALSE; }
	virtual BOOL OnNotifyFromParent(UINT /* nCode*/, LPNMHDR /*lpNMHDR*/) { return FALSE; }
	virtual BOOL OnSize(UINT_PTR /*uType*/, w32Rect& /*r*/) { return FALSE; }
	virtual void OnPaint(HDC hDC) {}	// Set WINDOW_FLAGS_CRACKPAINT to enable
	virtual DWORD OnCreate() { return ERROR_SUCCESS;  }			// Set WINDOW_FLAGS_CRACKCREATE to enable
	virtual void OnTimer(WPARAM id) {}	// Set WINDOW_FLAGS_CRACKTIMER to enable

	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	Window* GetParent()			{	return m_parent;	}
	void SetParent(HWND hWnd)	{	m_parent = (Window*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);	}

	void AddWindowStyles(DWORD dwStyle, DWORD dwExStyle = 0) {
		dwStyle |= GetWindowLongPtr(m_hWnd, GWL_STYLE);
		dwExStyle |= GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);
		SetWindowLongPtr(m_hWnd, GWL_STYLE, dwStyle);
		SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, dwExStyle);
	}

	void RemoveWindowStyles(DWORD dwStyle, DWORD dwExStyle = 0) {
		dwStyle = (DWORD)GetWindowLongPtr(m_hWnd, GWL_STYLE) & ~dwStyle;
		dwExStyle = (DWORD)GetWindowLongPtr(m_hWnd, GWL_EXSTYLE) & ~dwExStyle;
		SetWindowLongPtr(m_hWnd, GWL_STYLE, dwStyle);
		SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, dwExStyle);
	}

	void RedrawWindow(bool erase = true) {
		::RedrawWindow(*this, NULL, NULL, RDW_INVALIDATE | (erase ? RDW_ERASE: 0));
	}

	void SetRect(const RECT& r) {
		::SetWindowPos(*this, NULL, r.left, r.top, r.right - r.left + 1, r.bottom - r.top + 1, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	void ForceOnSize() {
		w32Rect r;
		::GetClientRect(*this, r);
//		if (m_wFlags & WINDOW_FLAGS_ADJUSTSIZE) {
//			AdjustClientRect(r);
//		}
		::SendMessage(*this, WM_SIZE, SIZE_RESTORED, LPARAM(r.Size()));
//		OnSize(SIZE_RESTORED, r);
	}

	void Center(Window* parent = NULL) {	// NULL = desktop
		w32Rect rParent;
		::GetWindowRect(parent ? *parent : ::GetDesktopWindow(), rParent);
		w32Rect rDialog;
		::GetWindowRect(*this, rDialog);

		rParent.left += (rParent.Width() - rDialog.Width()) / 2;
		rParent.top += (rParent.Height() - rDialog.Height()) / 2;
		::SetWindowPos(*this, NULL, rParent.left, rParent.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

	wchar_t* AllocWindowText() {
		int len = GetWindowTextLength(*this);
		wchar_t* sz = new wchar_t[(size_t)len + 1];
		::GetWindowText(*this, sz, len + 1);
		return sz;
	}

	// Static helpers
	static HINSTANCE GetInstance() { return m_hInstance; }
	static void ReportError(DWORD dw) {
		std::wstring str;
		w32GetError(str, dw);
		::MessageBox(NULL, str.c_str(), L"Error", MB_OK);
	}

	int Run(const Accelerator& acc = Accelerator(0));	// I get all the accel keypress messages, no matter where they come from
	static DWORD Register(LPCWSTR lpszClassName, std::function<void(WNDCLASSEX&)> fRegClassOpts);

	void ClientToScreen(RECT* r) {
		::ClientToScreen(*this, (LPPOINT)&r->left);
		::ClientToScreen(*this, (LPPOINT)&r->right);
	}

	void ScreenToClient(RECT* r) {
		::ScreenToClient(*this, (LPPOINT)&r->left);
		::ScreenToClient(*this, (LPPOINT)&r->right);
	}

	bool AreFlagsSet(WORD w) {
		return m_wFlags & w;
	}

protected:
	LRESULT CrackMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void SetHWnd(HWND hWnd) {	m_hWnd = hWnd;	}
	void SetFlag(WORD w, BOOL b) { 
		if (b)
			m_wFlags |= w; 
		else
			m_wFlags &= ~w; 
	}

private:
	HWND m_hWnd;
	static HINSTANCE m_hInstance;
	Window* m_parent;
	WORD m_wFlags;
};

// A Window with an app windows message that can be posted to it
class AWMNotifyTarget : public NotifyTarget
{
public:
	AWMNotifyTarget(Window* pWindow) : m_pWindow(pWindow) {}
	AWMNotifyTarget(Window* pWindow, UINT msg) : m_pWindow(pWindow), m_appMessage(msg) {}

	LRESULT NotifySend(WPARAM wParam, LPARAM lParam = 0) override {
		return ::SendMessage(*m_pWindow, m_appMessage, wParam, lParam);
	}

	void NotifyPost(WPARAM wParam, LPARAM lParam = 0) override {
		::PostMessage(*m_pWindow, m_appMessage, wParam, lParam);
	}

	operator UINT() { return m_appMessage; }

protected:
	Window* m_pWindow;
	AppMessage m_appMessage;
};

class AWMNList
{
public:
	void Add(Window* p, UINT msg) {
		m_list.push_back(AWMNotifyTarget(p, msg));
	}

	void NotifyPost(WPARAM w = 0, LPARAM l = 0) {
		for (auto& n : m_list) {
			n.NotifyPost(w, l);
		}
	}

protected:
	std::vector<AWMNotifyTarget> m_list;
};

class FullScreen
{
public:
	FullScreen(Window* p) : m_w(p), m_dwStyle(0), m_dwExStyle(0) {}

	void SetFullScreen(bool b) {
		if (b == IsFullScreen())
			return;

		if (b) {
			m_dwStyle = (DWORD)::GetWindowLongPtr(*m_w, GWL_STYLE);
			m_dwExStyle = (DWORD)::GetWindowLongPtr(*m_w, GWL_EXSTYLE);
			GetWindowRect(*m_w, m_rect);
			::SetWindowLongPtr(*m_w, GWL_STYLE, WS_POPUP | WS_VISIBLE);
			::SetWindowLongPtr(*m_w, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_TOPMOST);
			::SetWindowPos(*m_w, NULL, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_NOZORDER);
		}
		else {
			::SetWindowLongPtr(*m_w, GWL_STYLE, m_dwStyle);
			::SetWindowLongPtr(*m_w, GWL_EXSTYLE, m_dwExStyle);
			m_dwStyle = m_dwExStyle = 0;
			m_w->SetRect(m_rect);
		}
	}

	bool IsFullScreen() { return m_dwStyle || m_dwExStyle; }

protected:
	Window* m_w;
	DWORD m_dwStyle;
	DWORD m_dwExStyle;
	w32Rect m_rect;
};
