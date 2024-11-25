#include "Window.h"
#include "WindowExt.h"

void ScrollBarExt::Set() {
	if (!m_pOwner)	return;
	// Set the vertical scrolling range and page size
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE | SIF_PAGE;
	si.nMin = 0;
	si.nMax = m_range;
	si.nPage = m_page;
	::SetScrollInfo(*m_pOwner, m_bar, &si, TRUE);
}

int ScrollBarExt::GetPos() {
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	::GetScrollInfo(*m_pOwner, m_bar, &si);
	return si.nPos;
}

void ScrollBarExt::SetPos(int x) {
	// Set the vertical scrolling range and page size
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	si.nPos = x;
	::SetScrollInfo(*m_pOwner, m_bar, &si, TRUE);
}

void ScrollBarExt::OnMouseWheel(int16_t distance, WORD vkeys) {
	// If Shift key held, scroll horizontally.
	int sb = (vkeys & MK_SHIFT) ? SB_HORZ : SB_VERT;

	if (sb != m_bar) {
		return;
	}

	SCROLLINFO si;
	::ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	::GetScrollInfo(*m_pOwner, sb, &si);
	si.nPos -= distance;
	::SetScrollInfo(*m_pOwner, sb, &si, TRUE);

	m_pOwner->RedrawWindow((m_flags & FLAGS_DONT_CLEAR_ON_INVALIDATE) == 0);
}


LRESULT ScrollBarExt::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM) {
	if (message == WM_MOUSEWHEEL) {
		OnMouseWheel(HIWORD(wParam), LOWORD(wParam));
	}
	else if (message == m_message) {
		SCROLLINFO si;
		// Get all the vertial scroll bar information.
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		::GetScrollInfo(hWnd, m_bar, &si);

		// Save the position for comparison later on.
		int yPos = si.nPos;
		switch (LOWORD(wParam))
		{
			// User clicked the HOME keyboard key.
		case SB_TOP:
			si.nPos = si.nMin;
			break;

			// User clicked the END keyboard key.
		case SB_BOTTOM:
			si.nPos = si.nMax;
			break;

			// User clicked the top arrow.
		case SB_LINEUP:
			si.nPos -= m_lineClick;
			break;

			// User clicked the bottom arrow.
		case SB_LINEDOWN:
			si.nPos += m_lineClick;
			break;

			// User clicked the scroll bar shaft above the scroll box.
		case SB_PAGEUP:
			si.nPos -= m_pageClick;
			break;

			// User clicked the scroll bar shaft below the scroll box.
		case SB_PAGEDOWN:
			si.nPos += m_pageClick;
			break;

			// User dragged the scroll box.
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;

		default:
			break;
		}

		// Set the position and then retrieve it.  Due to adjustments
		// by Windows it may not be the same as the value set.
		si.fMask = SIF_POS;
		::SetScrollInfo(hWnd, m_bar, &si, TRUE);
		::GetScrollInfo(hWnd, m_bar, &si);

		// If the position has changed, scroll window and update it.
		if (si.nPos != yPos)
		{
			// ScrollWindow() would move the contents of the window across and then call
			// paint for the uncovered rectangle. We're just going to assume onpaint
			// repaints the whole window so we don't need the scrollwindow
			if ((m_flags & FLAGS_DONT_USE_SCROLLWINDOW) == 0) {
				::ScrollWindow(hWnd, 0, yPos - si.nPos, NULL, NULL);
			}

			// Redraw is called with false meaning the window isn't cleared by default
			m_pOwner->RedrawWindow((m_flags & FLAGS_DONT_CLEAR_ON_INVALIDATE) == 0);
		}
	}

	return 0;
}

LRESULT DoubleBufferExt::WndProc(HWND hWnd, UINT message, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	switch (message)
	{
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = ::BeginPaint(hWnd, &ps);

		// Get window size
		w32Rect rWindow;
		::GetClientRect(*m_pOwner, rWindow);
		int windowWidth = rWindow.Width();
		int windowHeight = rWindow.Height();

		// Create a memory bitmap to draw into - This eliminates flicker when refreshing very quickly.
		// The bitmap defaults to black so we need to ensure all of it is painted for it to look good.
		HDC hDCMem = ::CreateCompatibleDC(hdc);
		HBITMAP hBitmap = ::CreateCompatibleBitmap(hdc, windowWidth, windowHeight);
		HBITMAP hBitmapOld = (HBITMAP)::SelectObject(hDCMem, hBitmap);

		// Call the app OnPaint with the memory DC instead
		m_pOwner->OnPaint(hDCMem);

		// Copy the memory bitmap to the screen
		::BitBlt(hdc, 0, 0, windowWidth, windowHeight, hDCMem, 0, 0, SRCCOPY);

		// Clean up
		::SelectObject(hDCMem, hBitmapOld);
		::DeleteObject(hBitmap);
		::ReleaseDC(*m_pOwner, hDCMem);
		::EndPaint(hWnd, &ps);
		return 1;
	}
	}
	return 0;
}
