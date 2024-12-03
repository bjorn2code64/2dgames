#pragma once

#include <corecrt_math_defines.h>

#include <d2d1.h>

#include <Window.h>

class SizeF : public D2D1_SIZE_F
{
public:
	SizeF() { width = height = 0.0f; }
	SizeF(FLOAT w, FLOAT h) { width = w; height = h; }

	FLOAT Aspect() const { return (width == 0.0f) ? 1.0f : height / width; }
};

class Point2F : public D2D1_POINT_2F
{
public:
	Point2F() { x = y = 0.0f; }
	Point2F(FLOAT nx, FLOAT ny) { x = nx; y = ny; }
	Point2F(const w32Point& pt) { *this = pt; }
	Point2F operator*(FLOAT f) {
		return Point2F(x * f, y * f);
	}
	Point2F operator+=(const Point2F& rhs) {
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
	const Point2F& operator=(const w32Point& pt) {
		x = (FLOAT)pt.x;
		y = (FLOAT)pt.y;
		return *this;
	}
	double DistanceToSq(const Point2F& rhs) {
		return (rhs.x - x) * (rhs.x - x) + (rhs.y - y) * (rhs.y - y);
	}
	double DistanceTo(const Point2F& rhs) {
		return sqrt((rhs.x - x) * (rhs.x - x) + (rhs.y - y) * (rhs.y - y));
	}
	double angleradTo(const Point2F& rhs) const {
		return M_PI_2 + atan2(y - rhs.y, x - rhs.x);
	}
	bool Empty() { return x == 0.0f && y == 0.0f;  }
};

class RectF : public D2D1_RECT_F
{
public:
	RectF() { SetEmpty(); }
	RectF(FLOAT l, FLOAT t, FLOAT w, FLOAT h) { left = l; top = t; right = l + w; bottom = t + h; }

	Point2F TopLeft() { return Point2F(left, top); }
	Point2F TopRight() { return Point2F(right, top); }
	Point2F BottomLeft() { return Point2F(left, bottom); }
	Point2F BottomRight() { return Point2F(right, bottom); }

	bool ptInRect(const Point2F& pt) {
		return (pt.x >= left) && (pt.x <= right) && (pt.y >= top) && (pt.y <= bottom);
	}

	bool hitTest(const RectF& rhs) {
		if ((right < rhs.left) || (left > rhs.right)) return false;
		if ((bottom < rhs.top) || (top > rhs.bottom)) return false;
		return true;
	}

	operator D2D1_RECT_F* () { return this; }

	FLOAT Width() { return right - left; }
	FLOAT Height() { return bottom - top; }

	void GetCorners(std::vector<Point2F>& corners) {
		corners.push_back(TopLeft());
		corners.push_back(TopRight());
		corners.push_back(BottomLeft());
		corners.push_back(BottomRight());
	}

	void SetEmpty() {
		left = top = right = bottom = 0.0f;
	}

	bool IsEmpty() {
		return left == 0.0f && right == 0.0f && top == 0.0f && bottom == 0.0f;
	}

	void UnionRect(const RectF& rhs) {
		if (rhs.left < left) left = rhs.left;
		if (rhs.top < top) top = rhs.top;
		if (rhs.right > right) right = rhs.right;
		if (rhs.bottom > bottom) bottom = rhs.bottom;
	}

	void Offset(const Point2F& point) {
		left += point.x;
		right += point.x;
		top += point.y;
		bottom += point.y;
	}
};

class w32ColorF : public D2D1::ColorF {
public:
	w32ColorF(UINT32 rgb = 0) : D2D1::ColorF(rgb, 1.0F) {}

	w32ColorF& operator=(DWORD dw) {
		SetCOLORREF(dw);
		return *this;
	}

	operator int() {
		return (int)(r * 255.0F) << 16 |
			(int)(g * 255.0F) << 8 |
			(int)(b * 255.0F) << 0;
	}

	void SetCOLORREF(COLORREF bgr) {
		b = float((bgr & 0xff0000) >> 16) / 255.0F;
		g = float((bgr & 0x00ff00) >> 8) / 255.0F;
		r = float((bgr & 0x0000ff) >> 0) / 255.0F;
	}

	int GetCOLORREF() {
		return (int)(b * 255.0F) << 16 |
			(int)(g * 255.0F) << 8 |
			(int)(r * 255.0F) << 0;
	}
};

#define DEFAULT_DPI		96.0F

class D2DRectScaler
{
public:
	D2DRectScaler(long cx, long cy) : m_sizeBase(cx, cy), m_fScale(1.0f), m_gridSize(10),
		m_dpiX(DEFAULT_DPI), m_dpiY(DEFAULT_DPI)
	{}

	int GetGridSize() const { return m_gridSize; }
	void SetGridSize(int n) { m_gridSize = n; }

	void SetBaseSize(const w32Size& sz) {
		m_sizeBase = sz;
	}

	void SetBounds(const D2D1_SIZE_F& sizeAvail) {
		// sizeAvail is in actual DPI
		// We need to work out an X and Y offset and multiplier
		// such that a box of m_sizeVisible will fit in the m_size as large as possible
		FLOAT fScaleX = sizeAvail.width / (FLOAT)m_sizeBase.cx;
		FLOAT fScaleY = sizeAvail.height / (FLOAT)m_sizeBase.cy;
		m_fScale = (fScaleX < fScaleY) ? fScaleX : fScaleY;

		// Now work out the offsets
		FLOAT fMax = (FLOAT)m_sizeBase.cx * m_fScale;
		m_ptOffset.width = (sizeAvail.width - fMax) / 2;
		fMax = (FLOAT)m_sizeBase.cy * m_fScale;
		m_ptOffset.height = (sizeAvail.height - fMax) / 2;
	}

	void ReverseScaleAndOffset(w32Point* pt) const {
		PixToDPI(pt);

		pt->x = (LONG)(((FLOAT)pt->x - m_ptOffset.width) / m_fScale);
		pt->y = (LONG)(((FLOAT)pt->y - m_ptOffset.height) / m_fScale);
	}

	void ReverseScale(w32Point* pt) const {
		pt->x = (LONG)(((FLOAT)pt->x) / m_fScale);
		pt->y = (LONG)(((FLOAT)pt->y) / m_fScale);
	}

	FLOAT ScaleX(int x) const { return (FLOAT)x * m_fScale + m_ptOffset.width; }
	FLOAT ScaleY(int y) const { return (FLOAT)y * m_fScale + m_ptOffset.height; }

	void Scale(SizeF* pDest, const w32Point& ptSrc) const
	{
		pDest->width = ScaleX(ptSrc.x);
		pDest->height = ScaleY(ptSrc.y);
	}

	void Scale(Point2F* pDest, const w32Point& ptSrc) const
	{
		pDest->x = ScaleX(ptSrc.x);
		pDest->y = ScaleY(ptSrc.y);
	}

	void Scale(RectF* pDest, const w32Rect& src) const 
	{
		pDest->left = ScaleX(src.left);
		pDest->right = ScaleX(src.right);
		pDest->top = ScaleY(src.top);
		pDest->bottom = ScaleY(src.bottom);
	}

	void GetUserRect(RectF* pDest) const {
		Scale(pDest, w32Rect(0, 0, m_sizeBase.cx, m_sizeBase.cy));
	}

	void ScaleX(FLOAT* p) const { *p = *p * m_fScale + m_ptOffset.width; }
	void ScaleY(FLOAT* p) const { *p = *p * m_fScale + m_ptOffset.height; }
	void ScaleNoOffset(FLOAT* p) const { *p = *p * m_fScale; }

	void Scale(RectF* pDest) const
	{
		ScaleX(&pDest->left);
		ScaleX(&pDest->right);
		ScaleY(&pDest->top);
		ScaleY(&pDest->bottom);
	}

	void Scale(D2D1_POINT_2F* pDest) const
	{
		ScaleX(&pDest->x);
		ScaleY(&pDest->y);
	}

	void GridSnap(w32Point* pt) const {
		pt->x += m_gridSize - 1;
		pt->x -= pt->x % m_gridSize;

		pt->y += m_gridSize - 1;
		pt->y -= pt->y % m_gridSize;
	}

	FLOAT GetScaleX() const { return m_fScale * m_dpiX / DEFAULT_DPI; }
	FLOAT GetScaleY() const { return m_fScale * m_dpiY / DEFAULT_DPI; }

	void SetDPI(FLOAT x, FLOAT y) {
		m_dpiX = x; m_dpiY = y;
	}

	void PixToDPI(w32Point* pt) const {
		pt->x = (int)((double)pt->x * (double)DEFAULT_DPI / (double)m_dpiX);
		pt->y = (int)((double)pt->y * (double)DEFAULT_DPI / (double)m_dpiY);
	}

protected:
	w32Size m_sizeBase;
	FLOAT m_fScale;
	SizeF m_ptOffset;
	int m_gridSize;
	FLOAT m_dpiX;
	FLOAT m_dpiY;
};
