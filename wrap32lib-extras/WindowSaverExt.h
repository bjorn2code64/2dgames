#pragma once

#include <wrap32lib.h>
#include <windowext.h>

#include <FileHandler.h>

class WindowSaverExt : public WindowExt
{
public:
	WindowSaverExt(LPCWSTR appName) : m_appName(appName), m_pOwner(NULL) {}

	DWORD Init(Window* pOwner) {
		m_pOwner = pOwner;
		return ERROR_SUCCESS;
	}

protected:
	LRESULT WndProc(HWND hWnd, UINT message, WPARAM /*wParam*/, LPARAM lParam) override {
		switch (message) {
		case WM_CREATE: {
			// Load size and position
			IniFileHandler ifh(m_appName.c_str(), L"windowpos");
			int x = ifh.GetInt(L"pos", L"left", 0);
			int y = ifh.GetInt(L"pos", L"top", 0);
			int cx = ifh.GetInt(L"pos", L"width", 0);
			int cy = ifh.GetInt(L"pos", L"height", 0);
			if ((cx != 0) && (cy != 0)) {
				::SetWindowPos(hWnd, 0, x, y, cx, cy, SWP_NOZORDER);
			}
			break;
		}
		case WM_WINDOWPOSCHANGED: {
			WINDOWPOS* p = (WINDOWPOS*)lParam;
			m_pos = w32Point(p->x, p->y);
			m_size = w32Size(p->cx, p->cy);
			break;
		}
		case WM_CLOSE: {
			// Save size and position.
			IniFileHandler ifh(m_appName.c_str(), L"windowpos");
			ifh.SetInt(L"pos", L"left", m_pos.x);
			ifh.SetInt(L"pos", L"top", m_pos.y);
			ifh.SetInt(L"pos", L"width", m_size.cx);
			ifh.SetInt(L"pos", L"height", m_size.cy);
			break;
		}
		default:
			break;
		}
		return 0;

	}

protected:
	Window* m_pOwner;
	std::wstring m_appName;
	w32Point m_pos;
	w32Size m_size;
};
