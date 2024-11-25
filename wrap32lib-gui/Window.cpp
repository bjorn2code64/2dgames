#include "Window.h"

HINSTANCE Window::m_hInstance = NULL;

LRESULT CALLBACK lWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Window* p;
	if (message == WM_NCCREATE) {	// On creation, we get this (almost) first and the Window* passed in the CreateParams (We miss a WM_GETMINMAXINFO)
		// Set the Window pointer to the window
		p = (Window*)((CREATESTRUCT*)lParam)->lpCreateParams;
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)p);
	}
	else {
		p = (Window*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
	}

	LRESULT l = p ? p->WndProc(hWnd, message, wParam, lParam) : 0;
	return l ? l : ::DefWindowProc(hWnd, message, wParam, lParam);
}

DWORD Window::Register(LPCWSTR lpszClassName, std::function<void(WNDCLASSEX&)> fRegClassOpts)
{
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = lWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = m_hInstance;
	wcex.hIcon = NULL;			// LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WLBARE));
	wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;	// MAKEINTRESOURCEW(IDC_WLBARE);
	wcex.lpszClassName = lpszClassName;
	wcex.hIconSm = NULL;		// LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	if (fRegClassOpts) {
		fRegClassOpts(wcex);
	}

	return ::RegisterClassExW(&wcex) ? ERROR_SUCCESS : GetLastError();
}

DWORD Window::Create(Window* parent, LPCWSTR lpszClassName, LPCWSTR lpszWindowName, DWORD dwStyles,
		std::function<void(WNDCLASSEX&)> fRegClassOpts, int nID, DWORD dwExStyles)
{
	m_parent = parent;
	if (!(m_wFlags & WINDOW_FLAGS_DONTREGISTER))
		RETURN_IF_ERROR(Register(lpszClassName, fRegClassOpts));

	// We also set m_hWnd if/when we get the WM_CREATE message so it can be used to create
	// child windows.
	m_hWnd = ::CreateWindowEx(dwExStyles, lpszClassName, lpszWindowName, dwStyles,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		parent ? *parent : NULL, (HMENU)(UINT_PTR)nID, m_hInstance, (LPVOID)this);

	if (!m_hWnd) {
		return ::GetLastError();
	}

	// Now we have a wnd, we can init the extensions
	return InitExts(this);
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//  RETURN: 0 if unhandled to initiate a call to DefWndProc() default processing

LRESULT Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Let the extensions have a crack at the message first
	LRESULT l = WndProcExts(hWnd, uMsg, wParam, lParam);
	if (l)
		return l;

	switch (uMsg)
	{
	case WM_SIZE: {
		w32Rect r(0, 0, LOWORD(lParam) - 1, HIWORD(lParam) - 1);
		if (m_wFlags & WINDOW_FLAGS_ADJUSTSIZE) {
			AdjustClientRect(r);
		}

		if (OnSize((UINT_PTR)wParam, r))
			return 1;	// we handled it?
		break;
	}
	case WM_PAINT:
		if (m_wFlags & WINDOW_FLAGS_CRACKPAINT) {
			PAINTSTRUCT ps;
			HDC hdc = ::BeginPaint(hWnd, &ps);
			OnPaint(hdc);
			::EndPaint(hWnd, &ps);
		}
		break;
	case WM_CREATE:
		m_hWnd = hWnd;
		if (m_wFlags & WINDOW_FLAGS_CRACKCREATE) {
			DWORD dw = OnCreate();
			if (dw != ERROR_SUCCESS) {
				ReportError(dw);
				return -1;
			}
		}
		break;
	default:
		return CrackMessage(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

// Common messages between Window and Dialog
LRESULT Window::CrackMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_COMMAND:
		if (lParam) {	// Is there a control? e.g. buttons send events as a COMMAND, edits send notifications as WM_COMMAND
			HWND hWndFrom = HWND(lParam);

			Window* wChild = (Window*)::GetWindowLongPtr(hWndFrom, GWLP_USERDATA);;	// pass it down to the control that sent it
			if (wChild && (wChild->m_wFlags & WINDOW_FLAGS_MESSAGESFROMPARENT)) {
				LRESULT lr = wChild->OnCommandControlFromParent(int(HIWORD(wParam)), int(LOWORD(wParam)));
				if (lr)
					return lr;
			}

			if (m_wFlags & WINDOW_FLAGS_NOTIFYCOMMANDS) {
				// Upscale this to a notify
				NMHDR nmhdr;
				nmhdr.code = UINT(HIWORD(wParam));
				nmhdr.hwndFrom = hWnd;
				nmhdr.idFrom = UINT_PTR(LOWORD(wParam));

				if (OnNotify(hWndFrom, nmhdr.code, &nmhdr))
					return 1;
			}
			else {
				if (OnCommandControl(hWndFrom, int(HIWORD(wParam)), int(LOWORD(wParam))))
					return 1;
			}
		}
		else if ((HIWORD(wParam) == 0) || (HIWORD(wParam) == 1)) { // 0 menu - 1 accelerator
			if (OnCommandMenu(LOWORD(wParam))) {
				return 1;
			}

			// We may not be in a position to handle an accel. (common control, maybe)
			// Pass it to our parent if we have one
			if (m_parent)
				return m_parent->WndProc(hWnd, uMsg, wParam, lParam);
		}
	break;

	case WM_NOTIFY:
	{
		HWND hWndFrom = ((LPNMHDR)lParam)->hwndFrom;

		Window* wChild = (Window*)::GetWindowLongPtr(hWndFrom, GWLP_USERDATA);;	// pass it down to the control that sent it
		if (wChild && (wChild->m_wFlags & WINDOW_FLAGS_MESSAGESFROMPARENT)) {
			LRESULT lr = wChild->OnNotifyFromParent(((LPNMHDR)lParam)->code, (LPNMHDR)lParam);
			if (lr)
				return lr;
		}

		if (OnNotify(hWndFrom, ((LPNMHDR)lParam)->code, (LPNMHDR)lParam))	// We're the parent window - handle it
			return 1;
	}
	break;

	case WM_DESTROY:
		if (m_wFlags & WINDOW_FLAGS_QUITONCLOSE)
			::PostQuitMessage(0);
		break;

	case WM_TIMER:
		if (m_wFlags & WINDOW_FLAGS_CRACKTIMER) {
			OnTimer(wParam);
		}
		break;
	}

	return 0;
}

int Window::Run(const Accelerator& acc)
{
	// Main message loop:
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0))
	{
		if (!::TranslateAccelerator(m_hWnd, acc, &msg))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;	// On a WM_QUIT, we must return (from WinMain) the wParam value.
}

