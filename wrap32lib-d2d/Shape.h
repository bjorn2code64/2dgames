#pragma once

#include "d2dWindow.h"
#define _USE_MATH_DEFINES	// for M_PI
#include <math.h>

class Vector2F {
public:
	FLOAT x, y;

	Vector2F() : x(0.0F), y(0.0F) {}
	Vector2F(FLOAT x, FLOAT y) : x(x), y(y) {}

	Vector2F operator-(const Vector2F& other) const {
		return { x - other.x, y - other.y };
	}

	Vector2F operator+(const Vector2F& other) const {
		return { x + other.x, y + other.y };
	}

	Vector2F operator*(float scalar) const {
		return { x * scalar, y * scalar };
	}

	float dot(const Vector2F& other) const {
		return x * other.x + y * other.y;
	}

	Vector2F normalize() const {
		float length = std::sqrt(x * x + y * y);
		return { x / length, y / length };
	}

	float lengthsq() const {
		return (x * x) + (y * y);
	}

	double anglerad() const {
		if (y == 0) {
			return (x > 0) ? M_PI / 2 : 3 * M_PI / 2;
		}
		return M_PI / 2 + atan2(y, x);
	}
};

Vector2F reflect(const Vector2F& direction, const Vector2F& normal) {
	return direction - normal * (2 * direction.dot(normal));
}

////////////////////////////////////////////////////////////////////////
// Shape class manages position, direction, speed, color and
// alphablending using a solid brush and holds userdata and active
// status.
// Also has prototype functions for simple HitTesting.
// Shapes can be organised in a parent -> multiple child heirarchy.
//
////////////////////////////////////////////////////////////////////////

class Shape
{
public:
	static double RadToDeg(double rad) {
		return rad / M_PI * 180;
	}

	static double DegToRad(double deg) {
		return deg / 180 * M_PI;
	}

	enum class moveResult {
		ok,
		hitboundsright,
		hitboundsleft,
		hitboundstop,
		hitboundsbottom
	};

	Shape(const Point2F& pos, float speed, int direction, UINT32 rgb, FLOAT alpha = 1.0F, LPARAM userdata = 0) :
		m_pos(pos), m_fSpeed(speed), m_parent(NULL), m_pBrush(NULL), m_rgb(rgb), m_alpha(alpha), m_userdata(userdata), m_active(true)
	{
		SetDirectionInDeg(direction);
	}

	~Shape() {
		D2DDiscardResources();
		if (m_parent) {
			m_parent->RemoveChild(this);
		}
	}

	Point2F GetPos(bool includeParent = true) const {
		Point2F ret = m_pos;
		if (includeParent && m_parent) {
			ret += m_parent->GetPos(includeParent);
		}
		return ret;
	}

	void SetPos(const Point2F& pos) { m_pos = pos; }
	void OffsetPos(const Point2F& pos) { m_pos += pos; }

	void SetDirectionInDeg(int directionInDeg) {
		m_direction = ((double)directionInDeg * M_PI) / 180.0;
		UpdateCache();
	}

	void SetDirectionInRad(double direction) {
		m_direction = direction;
		UpdateCache();
	}

	FLOAT GetSpeed() { return m_fSpeed; }

	void SetSpeed(float speed) {
		m_fSpeed = speed;
		UpdateCache();
	}

	virtual void MovePos(Point2F& pos, float len = 0.0) const
	{
		if (len > 0.0) {
			float movelen = sqrt(m_cacheStep.lengthsq());
			float fraction = len / movelen;
			pos.x += m_cacheStep.x * fraction;
			pos.y += m_cacheStep.y * fraction;
		}
		else {
			pos.x += m_cacheStep.x;
			pos.y += m_cacheStep.y;
		}

		if (m_parent) {
			m_parent->MovePos(pos, len);
		}
	}

	void Move() { 
		m_pos.x += m_cacheStep.x; 
		m_pos.y += m_cacheStep.y;
		for (auto& c : m_children) {
			c->Move();
		}
	}

	static double GetBounceX(double dir) {
		if ((dir >= M_PI / 2.0) && (dir < 3.0 * M_PI / 2.0)) {
			return M_PI / 2.0 + (3.0 * M_PI / 2.0) - dir;
		}
		return 2.0 * M_PI - dir;
	}

	static double GetBounceY(double dir) {
		if (dir <= M_PI) {
			return M_PI - dir;
		}
		return M_PI + (2.0 * M_PI) - dir;
	}

	void BounceX() {
		m_direction = GetBounceX(m_direction);
		UpdateCache();
	}

	void BounceY() {
		m_direction = GetBounceY(m_direction);
		UpdateCache();
	}

	void Offset(const Point2F& pos) {
		m_pos += pos;
	}

	double GetDirection() {
		return m_direction;
	}

	int GetDirectionInDeg() {
		return (int)RadToDeg(m_direction);
	}

	void AddDirection(double f) {
		m_direction += f;
		UpdateCache();
	}

	void SetColor(UINT32 rgb) { m_rgb = rgb; }
	COLORREF GetColor() { return m_rgb; }

	void SetAlpha(FLOAT alpha) { m_alpha = alpha; }
	FLOAT GetAlpha() { return m_alpha; }

	void SetUserData(LPARAM l) { m_userdata = l; }
	LPARAM GetUserData() { return m_userdata; }

	void SetActive(bool b) { m_active = b; }
	bool IsActive() { return m_active; }

	// Resources and drawing
	virtual void D2DOnCreateResources(IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* m_pIWICFactory, const D2DRectScaler* pRS) {
		UINT32 rgb =
			(m_rgb & 0x000000ff) << 16 |
			(m_rgb & 0x0000ff00) |
			(m_rgb & 0x00ff0000) >> 16;	// RGB => BGR

		pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(rgb, m_alpha), &m_pBrush);
	}

	virtual void D2DDiscardResources() {
		SafeRelease(&m_pBrush);
	}

	ID2D1SolidColorBrush* GetBrush() { return m_pBrush; }

	virtual void Draw(ID2D1HwndRenderTarget* pRenderTarget, const D2DRectScaler* pRS = NULL) {
		for (auto m : m_children) {
			if (m->IsActive())
				m->Draw(pRenderTarget, pRS);
		}
	}
	virtual void Draw(ID2D1HwndRenderTarget* pRenderTarget, Point2F pos, const D2DRectScaler* pRS = NULL) {
		for (auto m : m_children) {
			if (m->IsActive())
				m->Draw(pRenderTarget, pos, pRS);
		}
	}

	// Lookahead WillHitBounds() for checking collision with screen edges.
	virtual moveResult WillHitBounds(const w32Size& screenSize) {
		return WillHitBounds(screenSize, GetPos());
	}

	virtual moveResult WillHitBounds(const w32Size& screenSize, const Point2F& pos) {
		Point2F posCurr = pos;
		MovePos(posCurr);
		RectF r;
		GetBoundingBox(&r, posCurr);

		if (r.left < 0)					return moveResult::hitboundsleft;
		if (r.right >= screenSize.cx)	return moveResult::hitboundsright;
		if (r.top < 0)					return moveResult::hitboundstop;
		if (r.bottom >= screenSize.cy)	return moveResult::hitboundsbottom;

		for (auto& c : m_children) {
			auto ret = c->WillHitBounds(screenSize);
			if (ret != moveResult::ok)
				return ret;
		}
		return moveResult::ok;
	}

	// Routine to get the bounding box for simple rect/rect hit testing
	virtual void GetBoundingBox(RectF*, const Point2F& pos) const {}

	// Hit testing with point, rect, shape
	virtual bool HitTest(Point2F pos) {
		RectF rThis;
		GetBoundingBox(&rThis, GetPos());
		return rThis.ptInRect(pos);
	}
	virtual bool HitTest(RectF rect) {
		RectF rThis;
		GetBoundingBox(&rThis, GetPos());
		return rThis.hitTest(rect);
	}
	bool HitTestShape(const Shape& rhs) {
		RectF rThis;
		GetBoundingBox(&rThis, GetPos());

		RectF rThat;
		rhs.GetBoundingBox(&rThat, rhs.GetPos());
		return rThis.hitTest(rThat);
	}

	// Heirarchy support
	void SetParent(Shape* group) { m_parent = group; }
	Shape* GetParent() { return m_parent; }

	void InsertChild(Shape* p) { m_children.insert(m_children.begin(), p); p->SetParent(this); }	// at the beginning
	void AddChild(Shape* p) { m_children.push_back(p); p->SetParent(this); }						// at the end
	void RemoveChild(Shape* p) {
		auto it = std::find(m_children.begin(), m_children.end(), p);
		if (it != m_children.end()) {
			m_children.erase(it);
		}
	}
	const std::vector<Shape*>& GetChildren() { return m_children; }

	void RemoveAllChildren(bool del = false) {
		if (del) {
			while (!m_children.empty()) {
				delete m_children.front();
			}
		}
		else {
			for (auto m : m_children) {
				Point2F p = m->GetPos();
				m->SetParent(NULL);
				m->SetPos(p);
			}
			m_children.clear();
		}
	}


protected:
	void UpdateCache() {
		//		char buff[256];
		//		snprintf(buff, 256, "deg %f rad %f\n", RadToDeg(m_direction), m_direction);
		//		OutputDebugStringA(buff);
		m_cacheStep = Vector2F((FLOAT)sin(m_direction), (FLOAT)-cos(m_direction)) * m_fSpeed;
	}

protected:
	Point2F m_pos;			// Current position
	double m_direction;		// Held in radians
	FLOAT m_fSpeed;			// Movement speed
	Vector2F m_cacheStep;	// Cache the move vector so we're not doing constant Trig
	ID2D1SolidColorBrush* m_pBrush;
	UINT32 m_rgb;
	FLOAT m_alpha;
	LPARAM m_userdata;
	bool m_active;

	// Heirarchy support
	std::vector<Shape*> m_children;
	Shape* m_parent;
};
