#pragma once

#include <wrap32lib.h>
#include <windowsx.h>

class StringTableEntry
{
public:
	StringTableEntry(int nID) : m_nID(nID), m_sz(NULL) { Load(); }
	~StringTableEntry() { if (m_sz)	delete[] m_sz; }

	const StringTableEntry& operator=(int nID) { m_nID = nID; return *this; }
	operator LPCWSTR () const;
	BOOL Get(LPCWSTR& sz);

protected:
	BOOL Load();

protected:
	LPWSTR m_sz;
	int m_nID;
};

class Accelerator
{
public:
	Accelerator(int nID);
	~Accelerator() {
		DestroyAcceleratorTable(m_h);
	}

	operator HACCEL() const { return m_h; }
protected:
	HACCEL m_h;
};

class w32Point : public POINT
{
public:
	w32Point()					{ x = y = 0; }
	w32Point(LPARAM lParam)		{ x = GET_X_LPARAM(lParam); y = GET_Y_LPARAM(lParam); }
	w32Point(SIZE size)			{ x = size.cx; y = size.cy; }
	w32Point(LONG xt, LONG yt)	{ x = xt; y = yt;	}

	POINT& operator=(const POINT& rhs) {
		x = rhs.x;
		y = rhs.y;
		return *this;
	}

	POINT& operator+=(const POINT& rhs) {
		return *this = *this + rhs;
	}

	POINT& operator-=(const POINT& rhs) {
		return *this = *this - rhs;
	}

	bool operator==(const POINT& rhs) const {
		return (x == rhs.x) && (y == rhs.y);
	}

	POINT operator+(const POINT& rhs) const {
		POINT pt;
		pt.x = x + rhs.x;
		pt.y = y + rhs.y;
		return pt;
	}

	POINT operator-(const POINT& rhs) const {
		POINT pt;
		pt.x = x - rhs.x;
		pt.y = y - rhs.y;
		return pt;
	}

	POINT operator*(double d) {
		POINT pt;
		pt.x = (int)(x * d + 0.5);
		pt.y = (int)(y * d + 0.5);
		return pt;
	}
};

class w32Size : public SIZE
{
public:
	w32Size()					{ cx = cy = 0; }
	w32Size(long _cx, long _cy)	{ cx = _cx; cy = _cy; }
	w32Size(POINT pt)			{ cx = pt.x; cy = pt.y; }
	w32Size(SIZE sz)			{ cx = sz.cx; cy = sz.cy; }
	w32Size(LPARAM lParam)		{ cx = GET_X_LPARAM(lParam);	cy = GET_Y_LPARAM(lParam); }

	float Aspect() const { return (cx == 0) ? 1.0f : (float)cy / (float)cx; }

	w32Size& operator=(const w32Point& pt) {
		cx = pt.x;
		cy = pt.y;
		return *this;
	}

	bool operator==(const SIZE& rhs) const {
		return (cx == rhs.cx) && (cy == rhs.cy);
	}

	SIZE operator*(double d) {
		SIZE sz;
		sz.cx = (int)(cx * d);
		sz.cy = (int)(cy * d);
		return sz;
	}

	operator LPARAM() { return MAKELPARAM(cx, cy); }
};

class w32Rect : public RECT
{
public:
	w32Rect() { ::SetRectEmpty(this); }
	w32Rect(int l, int t, int r, int b) { ::SetRect(this, l, t, r, b); }
	w32Rect(int l, int t, const w32Size& size) { ::SetRect(this, l, t, l + size.cx - 1, t + size.cy - 1); }
	w32Rect(int l, int t, const w32Point& pt) { ::SetRect(this, l, t, l + pt.x - 1, t + pt.y - 1); }
	w32Rect(const w32Point& pt, const w32Size& size) { ::SetRect(this, pt.x, pt.y, pt.x + size.cx - 1, pt.y + size.cy - 1); }
	w32Rect(int w, int h) { ::SetRect(this, 0, 0, w - 1, h - 1); }

	void Set(int l, int t, int r, int b) { ::SetRect(this, l, t, r, b); }

	void SetPos(int x, int y) { left = x; top = y; }
	void SetPos(POINT pt) { left = pt.x; top = pt.y; }
	void SetSize(int w, int h) { right = left + w - 1; bottom = top + h - 1; }

	operator RECT* () { return this; }

	int Width() const { return right - left + 1; }
	int Height() const { return bottom - top + 1; }

	int MidX() const { return (left + right) / 2; }
	int MidY() const { return (top + bottom) / 2; }

	w32Size Size() const { return w32Size(right - left + 1, bottom - top + 1); }

	w32Point TopLeft() const { return w32Point(left, top); }
	w32Point BottomRight() const { return w32Point(right, bottom); }

	float Aspect() const { return (Width() == 0) ? 1.0f : (float)Height() / (float)Width(); }

	void Expand(int l, int t, int r, int b) {
		left -= l;
		right += r;
		top -= t;
		bottom += b;
	}

	void Resize(double d) {
		right = (int)(left + Width() * d);
		bottom = (int)(top + Height() * d);
	}

	void Offset(int x, int y) {
		left += x;
		right += x;
		top += y;
		bottom += y;
	}
	void Offset(const w32Point& pt) {		Offset(pt.x, pt.y);	}

	void Move(int x, int y) {
		Offset(x - left, y - top);
	}
	void Move(const w32Point& pt) { Move(pt.x, pt.y); }

	bool ptInRect(const POINT& pt) {
		return (pt.x >= left) && (pt.x <= right) && (pt.y >= top) && (pt.y <= bottom);
	}
};
