#include "WLSyslink.h"

static WLSyslinkReg reg;

DWORD WLSyslink::CreateAndShow(Window* parent, LPCWSTR szText, int nCmdShow) {
	RETURN_IF_ERROR(Window::Create(parent, L"WLSYSLINK", szText, WS_CHILD));

	NONCLIENTMETRICS metrics;
	metrics.cbSize = sizeof(NONCLIENTMETRICS);
	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0);
	metrics.lfMessageFont.lfUnderline = TRUE;
	m_hFontUnderline = ::CreateFontIndirect(&metrics.lfMessageFont);

	RemoveWindowStyles(WS_BORDER);
	Show(nCmdShow);
	return ERROR_SUCCESS;
}

void WLSyslink::OnMouseMove(WORD, POINT pt) {
	if (GetCapture()) {
		w32Rect r;
		::GetClientRect(*this, r);
		if ((pt.x < 0) || (pt.x > r.right) || (pt.y < 0) || (pt.y > r.bottom)) {
			m_bHovered = FALSE;
			ReleaseCapture();
			RedrawWindow();
		}
	}
	else {
		m_bHovered = TRUE;
		SetCapture(*this);
		RedrawWindow();
	}
}

LRESULT WLSyslink::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_MOUSEMOVE:
		OnMouseMove((WORD)wParam, w32Point(lParam));
		break;

	case WM_LBUTTONDOWN: {
			wchar_t* text = AllocWindowText();
			::ShellExecute(NULL, L"open", text, NULL, NULL, SW_SHOW);
			delete[] text;
		}
		break;
	}

	return Window::WndProc(hWnd, message, wParam, lParam);
}

void WLSyslink::OnPaint(HDC hDC) {
	HFONT hFontOld = (HFONT)::SelectObject(hDC, m_hFontUnderline);

	SetTextColor(hDC, RGB(0, 0, 0xCC));
	SetBkMode(hDC, TRANSPARENT);

	wchar_t* text = AllocWindowText();

	w32Rect r;
	::GetClientRect(*this, r);
	::DrawText(hDC, text, static_cast<int>(wcslen(text)), &r, DT_LEFT | DT_SINGLELINE);
	delete[] text;

	if (m_bHovered)
		DrawFocusRect(hDC, &r);

	SetBkMode(hDC, OPAQUE);
	SelectObject(hDC, hFontOld);
}