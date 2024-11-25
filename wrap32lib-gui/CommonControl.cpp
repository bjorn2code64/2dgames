#include "CommonControl.h"

#pragma comment(lib, "comctl32")

// Ensure we link with common controls 6.0+ to support visual styles
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace CommonControls {

	BOOL CommonControl::LibInit()
	{
		INITCOMMONCONTROLSEX icex;           // Structure for control initialization.
		icex.dwSize = sizeof(icex);
		icex.dwICC = ICC_WIN95_CLASSES | ICC_DATE_CLASSES | ICC_INTERNET_CLASSES;
		return InitCommonControlsEx(&icex);
	}


	static LRESULT CALLBACK GCCWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
	{
		LRESULT p = ((CommonControl*)dwRefData)->WndProc(hWnd, uMsg, wParam, lParam);
		return p ? p : ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}

	DWORD CommonControl::Create(Window* parent, LPCWSTR lpszClassName, LPCWSTR lpszWindowName, DWORD dwStyles, int nID, DWORD dwExStyles)
	{
		RETURN_IF_ERROR(Window::Create(parent, lpszClassName, lpszWindowName, dwStyles, nullptr, nID, dwExStyles));
		return SubClass(TRUE);
	}

	DWORD CommonControl::SubClass(BOOL bDoit)
	{
		if (bDoit) {
			if (!::SetWindowSubclass(*this, GCCWndProc, NULL, DWORD_PTR(this))) {
				return GetLastError();
			}

			::SetWindowLongPtr(*this, GWLP_USERDATA, (LONG_PTR)this);
		}
		else {
			if (!::RemoveWindowSubclass(*this, GCCWndProc, DWORD_PTR(this))) {
				OutputDebugString(L"Warning: Didn't remove subclass\n");
			}

			::SetWindowLongPtr(*this, GWLP_USERDATA, (LONG_PTR)NULL);
		}

		return ERROR_SUCCESS;
	}

}