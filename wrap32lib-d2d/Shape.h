#pragma once

#include "d2dWindow.h"
#include "SS2DBrush.h"
#define _USE_MATH_DEFINES	// for M_PI
#include <math.h>

#define SHAPEDRAW_SHOW_GROUP_BOUNDS		0x0001
#define SHAPEDRAW_SHOW_BITMAP_BOUNDS	0x0002

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
// Shape class manages position, direction, speed and holds userdata,
// active status and an association to a brush.
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

	Shape(FLOAT x, FLOAT y, FLOAT speed, int direction, SS2DBrush* brush, LPARAM userdata = 0) :
		m_pos(x, y), m_fSpeed(speed), m_parent(NULL), m_pBrush(brush), m_userdata(userdata), m_active(true),
		m_childHasMoved(true)
	{
		SetDirectionInDeg(direction);
	}

	~Shape() {
	}

	Point2F GetPos(bool includeParent = true) const {
		Point2F ret = m_pos;
		if (includeParent && m_parent) {
			ret += m_parent->GetPos(includeParent);
		}
		return ret;
	}

	void SetPos(const Point2F& pos) {
		m_pos = pos;
		if (m_parent) {
			m_parent->ChildHasMoved();
		}
	}
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
		if (m_parent) {
			m_parent->ChildHasMoved();
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

	void SetUserData(LPARAM l) { m_userdata = l; }
	LPARAM GetUserData() { return m_userdata; }

	void SetActive(bool b) {
		m_active = b;
		if (m_parent) {
			m_parent->ChildHasMoved();
		}
	}

	bool IsActive() { return m_active; }

	virtual void SS2DCreateResources(IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* m_pIWICFactory, const D2DRectScaler* pRS) {
	}

	virtual void SS2DDiscardResources() {
	}

	void SetBrush(SS2DBrush* b) { m_pBrush = b;  }
	SS2DBrush* GetBrush() { return m_pBrush; }
	COLORREF GetBrushColor() { return m_pBrush ? m_pBrush->GetColor() : 0; }

	virtual void Draw(ID2D1HwndRenderTarget* pRenderTarget, DWORD dwFlags, const D2DRectScaler* pRS = NULL) {
		for (auto m : m_children) {
			if (m->IsActive())
				m->Draw(pRenderTarget, dwFlags, pRS);
		}
	}
	virtual void Draw(ID2D1HwndRenderTarget* pRenderTarget, Point2F pos, DWORD dwFlags, const D2DRectScaler* pRS = NULL) {
		for (auto m : m_children) {
			if (m->IsActive())
				m->Draw(pRenderTarget, pos, dwFlags, pRS);
		}
	}

	// Lookahead WillHitBounds() for checking collision with screen edges.
	virtual moveResult WillHitBounds(const w32Size& screenSize) {
		return WillHitBounds(RectF(0, 0, (FLOAT)screenSize.cx, (FLOAT)screenSize.cy), GetPos());
	}

	virtual moveResult WillHitBounds(const w32Size& screenSize, const Point2F& pos) {
		return WillHitBounds(RectF(0, 0, (FLOAT)screenSize.cx, (FLOAT)screenSize.cy), pos);
	}

	virtual moveResult WillHitBounds(const RectF& rBounds) {
		return WillHitBounds(rBounds, GetPos());
	}

	virtual moveResult WillHitBounds(const RectF& rBounds, const Point2F& pos) {
		Point2F posCurr = pos;
		MovePos(posCurr);
		RectF r;
		GetBoundingBox(&r, posCurr);

		if (r.left < rBounds.left)			return moveResult::hitboundsleft;
		if (r.right >= rBounds.right)		return moveResult::hitboundsright;
		if (r.top < rBounds.top)			return moveResult::hitboundstop;
		if (r.bottom >= rBounds.bottom)		return moveResult::hitboundsbottom;

		for (auto& c : m_children) {
			auto ret = c->WillHitBounds(rBounds);
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
	bool HitTestShape(Shape* rhs) {
		RectF rThis;
		GetBoundingBox(&rThis, GetPos());

		RectF rThat;
		rhs->GetBoundingBox(&rThat, rhs->GetPos());
		return rThis.hitTest(rThat);
	}

	// Heirarchy support
	void SetParent(Shape* group) { m_parent = group; }
	Shape* GetParent() { return m_parent; }

	void InsertChild(Shape* p) { m_children.insert(m_children.begin(), p); p->SetParent(this); }	// at the beginning
	void AddChild(Shape* p) { m_children.push_back(p); p->SetParent(this); }						// at the end
	void RemoveChild(Shape* p, bool del = false) {
		auto it = std::find(m_children.begin(), m_children.end(), p);
		if (it != m_children.end()) {
			if (del) {
				delete* it;
			}
			m_children.erase(it);
		}
		ChildHasMoved();
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

	void ChildHasMoved() {
		m_childHasMoved = true;
	}

protected:
	void UpdateCache() {
		m_cacheStep = Vector2F((FLOAT)sin(m_direction), (FLOAT)-cos(m_direction)) * m_fSpeed;
	}

protected:
	Point2F m_pos;			// Current position
	double m_direction;		// Held in radians

	FLOAT m_fSpeed;			// Movement speed
	Vector2F m_cacheStep;	// Cache the move vector so we're not doing constant Trig

	// Colour
	SS2DBrush* m_pBrush;

	// Is it drawn?
	bool m_active;

	// User settable id
	LPARAM m_userdata;

	// Heirarchy support
	Shape* m_parent;
	std::vector<Shape*> m_children;
	bool m_childHasMoved;
};
