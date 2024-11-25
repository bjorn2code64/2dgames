#pragma once

#include <wrap32lib.h>
#include <Window.h>
#include <WindowExt.h>

// A splitter management class - Displays a splitter and sends resize messages back to the owner
// with the left and right rectangle positions

// Some pixel sizes
#define SPLITTER_WIDTH	3
#define SPLITTER_BORDER	1

class Splitter : public WindowExt
{
public:
	Splitter(int initial = 500) : m_nDragOffsetX(-1), m_pOwner(NULL)
	{
		m_rect.right = (m_rect.left = initial) + SPLITTER_WIDTH;	// a default
		m_rect.top = 0;
	}

	int GetPos() { return m_rect.left; }
	void SetPos(int nPos)
	{
		if (nPos < 0)	nPos = 0;
		RECT rc;
		::GetWindowRect(*m_pOwner, &rc);
		int nMaxPos = rc.right - rc.left + 1 - SPLITTER_WIDTH;
		if (nPos > nMaxPos)	nPos = nMaxPos;

		m_rect.right = (m_rect.left = nPos) + SPLITTER_WIDTH;
	}

	AppMessage GetAppMessage() const {	return m_msg;	}

protected:
	void Paint()
	{
		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(*m_pOwner, &ps);
		::FillRect(hDC, &m_rect, GetSysColorBrush(COLOR_BTNFACE));
		w32Rect r = m_rect;
		r.left += SPLITTER_BORDER;
		r.right -= SPLITTER_BORDER;
		r.top += SPLITTER_BORDER;
		r.bottom -= SPLITTER_BORDER;
		::FillRect(hDC, &r, GetSysColorBrush(COLOR_BTNSHADOW));
		EndPaint(*m_pOwner, &ps);
	}

	DWORD Init(Window* pOwner) override {
		m_pOwner = pOwner;
		return ERROR_SUCCESS;
	}

	LRESULT WndProc(HWND /*hWnd*/, UINT message, WPARAM wParam, LPARAM lParam) override
	{
		switch (message)
		{
		case WM_PAINT: {
			Paint();
			break;
		}

		case WM_SIZE: {
			if (m_pOwner && (wParam != SIZE_MINIMIZED)) {
//				w32Size sz(lParam);
				w32Rect r(w32Point(0, 0), w32Size(lParam));
				if (m_pOwner->AreFlagsSet(WINDOW_FLAGS_ADJUSTSIZE)) {
					m_pOwner->AdjustClientRect(r);
				}

				// Update the position of our splitter for detection and drawing
				m_rect.top = r.top;
				m_rect.bottom = r.bottom;

				if (r.Width() - SPLITTER_WIDTH < m_rect.left)	// don't lose the splitter off the right
					SetPos(r.Width() - SPLITTER_WIDTH);

				// Send resize message to owner
				w32Rect rs[2];	// An array of 2 rects, left then right window positions relative to the owner window
				rs[0].Set(r.left, r.top, m_rect.left - 1, r.bottom);
				rs[1].Set(m_rect.right + 1, r.top, r.right, r.bottom);
				::SendMessage(*m_pOwner, (UINT)m_msg, wParam, (LPARAM)rs);	// also send wParam from OnSize?
				::InvalidateRect(*m_pOwner, m_rect, FALSE);
			}
			break;
		}

		case WM_SETCURSOR: {
			w32Point pt;
			GetCursorPos(&pt);
			::ScreenToClient(*m_pOwner, &pt);
			if (m_rect.ptInRect(pt)) {
				::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
				return TRUE;	// stop windows setting the cursor back to the default
			}
		}
		break;

		case WM_MOUSEMOVE: {
			if (m_nDragOffsetX != -1) {	// We're dragging
				w32Point pt(lParam);
				SetPos(pt.x - m_nDragOffsetX);	// adjust for the initial mouse X offset into the rect

				// Update stuff as the user drags
				m_pOwner->ForceOnSize();
			}
		}
		break;

		case WM_LBUTTONDOWN: {
			w32Point pt(lParam);
			if (m_rect.ptInRect(pt)) {
				// Start dragging and record the mouse X offset into the splitter rect
				m_nDragOffsetX = pt.x - m_rect.left;
				SetCapture(*m_pOwner);
			}
		}
		break;

		case WM_LBUTTONUP:
			if (m_nDragOffsetX != -1) {
				// Stop dragging
				m_nDragOffsetX = -1;
				ReleaseCapture();
			}
			break;

		default:
			break;
		}
		return 0;
	}

protected:
	Window* m_pOwner;
	w32Rect m_rect;
	int m_nDragOffsetX;
	AppMessage m_msg;
};

