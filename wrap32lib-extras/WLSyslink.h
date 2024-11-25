#pragma once

#include <Window.h>
#include <shellapi.h>

class WLSyslinkReg {

public:
	WLSyslinkReg() {
		DWORD dwRet = Window::Register(L"WLSYSLINK", [](WNDCLASSEX& wcex) {
			wcex.hbrBackground = HBRUSH(COLOR_3DFACE + 1);
		});
		if (dwRet != ERROR_SUCCESS)
			Window::ReportError(dwRet);
	}
};

class WLSyslink : public Window
{
public:
	WLSyslink() : Window(WINDOW_FLAGS_DONTREGISTER | WINDOW_FLAGS_CRACKPAINT), m_bHovered(FALSE), m_hFontUnderline(NULL) {}
	~WLSyslink() { if (m_hFontUnderline) ::DeleteFont(m_hFontUnderline); }

	DWORD CreateAndShow(Window* parent, LPCWSTR szText, int nCmdShow = SW_SHOW);
	void OnMouseMove(WORD, POINT pt);

protected:
	LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
	void OnPaint(HDC hDC) override;

protected:
	BOOL m_bHovered;
	HFONT m_hFontUnderline;
};