#pragma once

#include <wrap32lib.h>
#include <Window.h>
#include <CommonControl.h>
#include <GDI.h>

#define GVN_GETDISPINFO		1
#define GVN_ODCACHEHINT		2
#define GVN_ITEMCHANGED		3

#define GM_RCLICK			4

struct NMGVDISPINFO {
	NMHDR hdr;
	int row;
	int col;
	LPCWSTR pszText;	// return value
};

struct NMGVCACHEHINT {
	NMHDR hdr;
	int colStart;
	int colEnd;
	int rowStart;
	int rowEnd;
};

struct NMGRIDVIEW {
	NMHDR  hdr;
	int    row;
	int    col;
//	UINT   uNewState;
//	UINT   uOldState;
//	UINT   uChanged;
//	POINT  ptAction;
//	LPARAM lParam;
};

struct NMGVCELL {
	NMHDR  hdr;
	int    row;
	int    col;
};

class GridViewColumn
{
protected:
	const int min_width = 20;

public:
	GridViewColumn(LPCWSTR title, int width) : m_title(title), m_width(width), m_widthOffset(0)	{}

	LPCWSTR GetTitle() { return m_title.c_str(); }
	void SetTitle(LPCWSTR title) { m_title = title; }
	int GetWidth() { return m_width + m_widthOffset; }

	bool SetWidthOffset(int x) {
		if (m_width + x > min_width) {
			if (m_widthOffset == x) return false;	// no change
			m_widthOffset = x;
		}
		else {
			if (m_widthOffset == min_width - m_width) return false;	// no change
			m_widthOffset = min_width - m_width;
		}
		return true;	// changed
	}

	void FinaliseWidthOffset() {
		m_width += m_widthOffset;
		m_widthOffset = 0;
	}

protected:
	std::wstring m_title;
	int m_width;
	int m_widthOffset;	// For dragging/resizing
};

class GridView : public Window
{
public:
	const int rowHeight = 20;
	const int headerHeight = 20;
	const int horizontal_scroll_linesize = 10;

	GridView() : Window(WINDOW_FLAGS_NOTIFYCOMMANDS),
		m_rows(0),
		m_colDragging(-1),
		m_extScrollbarH(SB_HORZ, ScrollBarExt::FLAGS_DONT_CLEAR_ON_INVALIDATE | ScrollBarExt::FLAGS_DONT_USE_SCROLLWINDOW),
		m_extScrollbarV(SB_VERT, ScrollBarExt::FLAGS_DONT_CLEAR_ON_INVALIDATE | ScrollBarExt::FLAGS_DONT_USE_SCROLLWINDOW),
		m_cellSelected(-1, -1),
		m_brSelected(RGB(0, 120, 215)),
		m_brNull(NULL_BRUSH),
		m_brFill(GetSysColor(COLOR_BTNFACE)),
		m_brHeaderBorder(GetSysColor(COLOR_3DSHADOW)),
		m_penSelectedCellBorder(PS_DOT, 2, GetSysColor(COLOR_3DSHADOW)),
		m_brWindow(DC_BRUSH),
		m_font(L"Segoe UI", -12),
		m_nTopRow(0)
	{
		// Add in the double buffer support - it calls OnPaint when required
		AddExt(&m_extDoubleBuffer);

		// Add in scroll bar support
		m_extScrollbarH.SetLineClick(horizontal_scroll_linesize);
		AddExt(&m_extScrollbarH);

		m_extScrollbarV.SetLineClick(rowHeight);
		AddExt(&m_extScrollbarV);

		// Allocate resources
		m_hCursorResize = ::LoadCursor(NULL, IDC_SIZEWE);

		// Get stock resources
		m_colText = GetSysColor(COLOR_WINDOWTEXT);
		m_colTextSelected = GetSysColor(COLOR_WINDOW);
	}

	~GridView() {
		// Deallocate resources
		::DestroyCursor(m_hCursorResize);

		DeleteAllColumns();
	}

	DWORD CreateAndShow(Window* parent, int nCmdShow) {
		RETURN_IF_ERROR(Window::Create(parent, L"GridView", L"", WS_CHILDWINDOW | WS_HSCROLL | WS_VSCROLL));
		Show(nCmdShow);
		UpdateScrollBarRangesWindow();
		UpdateScrollBarRangesDoc();
//		RETURN_IF_ERROR(m_tt.CreateAndShow(this));
		return ERROR_SUCCESS;
	}

	void SetRows(int rows, bool reset = true) {
		m_rows = rows;
		if (reset) {
			m_extScrollbarH.SetPos(0);
			m_extScrollbarV.SetPos(0);
			m_cellSelected = w32Point(-1, -1);
			SendMessageToParent(GVN_ITEMCHANGED, m_cellSelected.y, m_cellSelected.x);
		}
		UpdateScrollBarRangesDoc();
		RedrawWindow(false);
	}

	void SetColumnTitle(int index, LPCWSTR title) {
		m_cols[index]->SetTitle(title);
	}

	LPCWSTR GetColumnTitle(int index) {
		return m_cols[index]->GetTitle();
	}

	void Reset() {
		DeleteAllColumns();
		SetRows(0);
	}
	void AppendColumn(LPCWSTR name, int width) {
		m_cols.push_back(new GridViewColumn(name, width));
		UpdateScrollBarRangesDoc();
		RedrawWindow(false);
	}

	const w32Point& GetCellSelected() { return m_cellSelected;  }

protected:
	void DeleteAllColumns() {
		for (auto p : m_cols) {
			delete p;
		}
		m_cols.clear();
		UpdateScrollBarRangesDoc();
		RedrawWindow(false);
	}

	void SendMessageToParent(int code, int row, int col) {
		NMGVCELL gvcell;
		gvcell.hdr.hwndFrom = *this;
		gvcell.hdr.code = code;
		gvcell.hdr.idFrom = 0;
		gvcell.row = row;
		gvcell.col = col;
		::SendMessage(*GetParent(), WM_NOTIFY, code, (LPARAM)&gvcell);
	}

	LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override {
		switch (message) {
			case WM_MOUSEMOVE: {
				OnMouseMove(w32Point(lParam));
				break;
			}
			case WM_LBUTTONDOWN: {
				OnLButtonDown(w32Point(lParam));
				break;
			}
			case WM_LBUTTONUP: {
				OnLButtonUp();
				break;
			}
			case WM_RBUTTONUP: {
				w32Point ptClick(lParam);
				OnButtonDown(ptClick);
				w32Point pt = HitTestCells(ptClick);
				SendMessageToParent(GM_RCLICK, pt.y, pt.x);
				break;
			}
		}
		return __super::WndProc(hWnd, message, wParam, lParam);
	}

	BOOL OnSize(UINT_PTR /*uType*/, w32Rect& r) override {
//		m_tt.OnSize(r);
		UpdateScrollBarRangesWindow();
		return FALSE;
	}

	////////////////////////////////////////
	// Column resize

	int HitTestColDivider(const w32Point& pt) {
		for (auto& m : m_dividers) {
			if (::PtInRect(m.first, pt)) {
				return m.second;
			}
		}
		return -1;
	}

	w32Point HitTestCells(const w32Point& pt) {
		// row, col
		w32Point ret(0, m_nTopRow + (pt.y - headerHeight) / rowHeight);

		// Work out the column based on the column widths
		int xpos = m_extScrollbarH.GetPos() + pt.x;
		for (auto pCol : m_cols) {
			xpos -= pCol->GetWidth();
			if (xpos < 0)
				break;
			ret.x++;
		}

		if (xpos >= 0) {	// clicked to the right of the last column
			ret.x = ret.y = -1;
		}

		if (pt.y - headerHeight < 0) {
			ret.y = -1;		// Clicked in the header - just return the column number
		}

		return ret;
	}

	void OnButtonDown(const w32Point& pt) {
		m_cellSelected = HitTestCells(pt);
		SendMessageToParent(GVN_ITEMCHANGED, m_cellSelected.y, m_cellSelected.x);
		RedrawWindow(false);
	}

	void OnLButtonDown(const w32Point& pt) {
		int colDividerSelected = HitTestColDivider(pt);
		if (colDividerSelected >= 0) {
			::SetCapture(*this);
			::SetCursor(m_hCursorResize);
			m_ptDrag = pt;
			m_colDragging = colDividerSelected;
		}
		else {
			OnButtonDown(pt);
		}
	}

	void OnMouseMove(const w32Point& pt) {
		if (m_colDragging != -1) {
			::SetCursor(m_hCursorResize);
			if (m_cols[m_colDragging]->SetWidthOffset(pt.x - m_ptDrag.x)) {
				UpdateScrollBarRangesDoc();
				RedrawWindow(false);
			}
		}
		else {
			int colDividerSelected = HitTestColDivider(pt);
			if (colDividerSelected >= 0) {
				::SetCursor(m_hCursorResize);
			}
		}
	}

	void OnLButtonUp() {
		if (m_colDragging != -1) {
			::SetCursor(m_hCursorResize);
			::ReleaseCapture();
			m_cols[m_colDragging]->FinaliseWidthOffset();
			m_colDragging = -1;
			UpdateScrollBarRangesDoc();
			RedrawWindow(false);
		}
	}

	////////////////////////////////////////
	// Scrolling

	// Window size changes
	void UpdateScrollBarRangesWindow() {
		w32Rect rWindow;
		::GetClientRect(*this, rWindow);
		const int windowWidth = rWindow.Width();
		const int windowHeight = rWindow.Height();

		m_extScrollbarH.SetPageClick(windowWidth);
		m_extScrollbarH.SetPage(windowWidth);

		m_extScrollbarV.SetPageClick(windowHeight - headerHeight);
		m_extScrollbarV.SetPage(windowHeight - headerHeight);
	}

	// Document size changes
	void UpdateScrollBarRangesDoc() {
		m_extScrollbarH.SetRange(GetDocWidth());
		m_extScrollbarV.SetRange(GetDocHeight());
	}

	int GetDocWidth() {	// Get the width of the whole "document"
		int x = 0;
		for (auto p : m_cols) {
			int width = p->GetWidth();
			x += width;
		}
		return x;
	}

	int GetDocHeight() {	// Get the height of the whole "document"
		return m_rows ? (rowHeight * m_rows) : headerHeight;
	}

	////////////////////////////////////////
	// Painting the window

	void OnPaint(HDC hDC) override {
		GDI gdi(hDC);

		// Get scroll window size
		w32Rect rWindow;
		::GetClientRect(*this, rWindow);
		int windowWidth = rWindow.Width();
		int windowHeight = rWindow.Height();
		int viewPortHeight = windowHeight - headerHeight;

		m_dividers.clear();

		// Set the resources up
		gdi.Select(m_font);
		gdi.SetBkMode(TRANSPARENT);

		// Get scroll offsets
		int x = -m_extScrollbarH.GetPos();
		int y = -m_extScrollbarV.GetPos();

		// Work out which columns are in view
		int colStart = (int)m_cols.size();
		int colEnd = -1;
		for (int col = 0; (col < m_cols.size()) && (x <= windowWidth); col++) {	// just go to the last visible column
			auto pCol = m_cols[col];
			int width = (int)pCol->GetWidth();
			if ((x >= 0) && (x <= windowWidth)) {	// Draw the column if we can see it
				if (col < colStart) {
					colStart = col;
				}
				if (col > colEnd) {
					colEnd = col;
				}
			}
			x += width;
		}

		// Work out which rows are in view
		m_nTopRow = m_rows;
		int rowEnd = -1;
		for (int row = 0; (row < m_rows) && (y <= viewPortHeight); row++) {
			if ((y >= 0) && (y <= viewPortHeight)) {
				if (row < m_nTopRow) {
					m_nTopRow = row;
				}
				if (row > rowEnd) {
					rowEnd = row;
				}
			}
			y += rowHeight;
		}

		// Send a cache hint
		Window* parent = GetParent();
		NMGVCACHEHINT ch;
		ch.hdr.code = GVN_ODCACHEHINT;
		ch.hdr.hwndFrom = *this;
		ch.hdr.idFrom = 0;
		ch.colStart = colStart;
		ch.colEnd = colEnd;
		ch.rowStart = m_nTopRow;
		ch.rowEnd = rowEnd;
		::SendMessage(*parent, WM_NOTIFY, GVN_ODCACHEHINT, (LPARAM)&ch);

		// Draw everything
		x = -m_extScrollbarH.GetPos();

		// Set up a structure to repeatedly ask for cell info with
		NMGVDISPINFO dispinfo;
		dispinfo.hdr.code = GVN_GETDISPINFO;
		dispinfo.hdr.hwndFrom = *this;
		dispinfo.hdr.idFrom = 0;

		// Iterate all the columns working out the position using the varying column widths
		for (int col = 0; col < m_cols.size(); col++) {
			auto pCol = m_cols[col];
			int width = (int)pCol->GetWidth();

			w32Rect r(x, headerHeight, x + width, headerHeight + rowHeight);
			if ((r.right >= 0) && (r.left <= windowWidth)) {	// Can we see the column in the potentially scrolled window

				DWORD colTextOld = ::SetTextColor(hDC, m_colText);

				// Draw the rows that are in view only
				for (int row = m_nTopRow; row <= rowEnd; row++) {
					// clear the background for this item
					bool rowSelected = row == m_cellSelected.y;
					::FillRect(hDC, r, rowSelected ? m_brSelected : m_brWindow);
					::SetTextColor(hDC, rowSelected ? m_colTextSelected : m_colText);

					// Ask for the contents of this cell
					dispinfo.row = row;
					dispinfo.col = col;
					dispinfo.pszText = NULL;
					::SendMessage(*parent, WM_NOTIFY, GVN_GETDISPINFO, (LPARAM)&dispinfo);
					if (dispinfo.pszText) {
						// If there are contents, draw them
						r.Expand(-5, -1, -5, -1);
						::DrawText(hDC, dispinfo.pszText, (int)wcslen(dispinfo.pszText), r, DT_LEFT | DT_BOTTOM | DT_END_ELLIPSIS);
						r.Expand(5, 1, 5, 1);
					}

					if (rowSelected && col == m_cellSelected.x) {
						// border the selected cell
						GDI gdiPen(hDC);
						gdiPen.Select(m_penSelectedCellBorder);
						gdiPen.Select(m_brNull);
						::Rectangle(hDC, r.left + 1, r.top + 1, r.right, r.bottom);
					}

					// Move onto the next row
					r.Offset(0, rowHeight);
				}

				::SetTextColor(hDC, colTextOld);

				// Draw the header at the top over whatever might be drawn there
				w32Rect rHeader(x, 0, x + width, headerHeight);
				::FillRect(hDC, rHeader, m_brFill);	// colour the background
				::FrameRect(hDC, rHeader, m_brHeaderBorder);		// draw a border around it
				rHeader.Expand(-5, -1, -5, -1);

				// Draw the column title
				LPCWSTR sz = pCol->GetTitle();
				::DrawText(hDC, sz, (int)wcslen(sz), rHeader, DT_LEFT | DT_BOTTOM | DT_END_ELLIPSIS);

				rHeader.Expand(5, 1, 5, 1);

				// Add a divider rect to the list for mouse pointer changing
				m_dividers.push_back(std::make_pair(w32Rect(rHeader.right - 3, rHeader.top, rHeader.right + 3, rHeader.bottom), col));
			}

			// Clear down to the bottom of the window
			r.bottom = rWindow.bottom;
			::FillRect(hDC, r, m_brWindow);

			x += width;
		}
		// Clear to the right of the window
		w32Rect rRight(x, 0, rWindow.right, rWindow.bottom);
		::FillRect(hDC, rRight, m_brWindow);
	}

protected:
	DoubleBufferExt m_extDoubleBuffer;
	ScrollBarExt m_extScrollbarH;
	ScrollBarExt m_extScrollbarV;

	std::vector<GridViewColumn*> m_cols;	// Column position, title, etc.
	int m_rows;								// Number of rows
	w32Point m_cellSelected;				// x/y position of the selected cell (or -1/-1 if nothing selected)
	int m_nTopRow;							// Track this to help hittesting - using the scrollbar seems to introduce errors

	std::vector<std::pair<w32Rect, int>> m_dividers;	// List of rectangles for hit testing column resize
	HCURSOR m_hCursorResize;				// Cursor to show when hitting a resize rectangle

	int m_colDragging;						// Index of column currently being resized or -1 if not happening
	w32Point m_ptDrag;						// Point where we started resizing a column

//	CommonControls::ToolTip m_tt;

	// Resources
	Font m_font;
	StockBrush m_brWindow;
	SolidBrush m_brHeaderBorder;
	SolidBrush m_brFill;
	SolidBrush m_brSelected;
	StockBrush m_brNull;
	Pen m_penSelectedCellBorder;
	DWORD m_colText;
	DWORD m_colTextSelected;
};
