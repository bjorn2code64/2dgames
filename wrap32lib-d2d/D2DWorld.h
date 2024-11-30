#pragma once

#include <d2dWindow.h>
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

class Position
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

	Position(const Point2F& pos, float speed, int direction) : m_pos(pos), m_fSpeed(speed)
	{
		SetDirectionInDeg(direction);
	}

	const Point2F& GetPos() const { return m_pos; }
	void SetPos(const Point2F& pos) { m_pos = pos; }
	void OffsetPos(const Point2F& pos) { m_pos += pos;  }

	void SetDirectionInDeg(int directionInDeg) {
		m_direction = ((double)directionInDeg * M_PI) / 180.0;
		UpdateCache();
	}

	void SetDirectionInRad(double direction) {
		m_direction = direction;
		UpdateCache();
	}

	FLOAT GetSpeed() { return m_fSpeed;  }

	void SetSpeed(float speed) {
		m_fSpeed = speed;
		UpdateCache();
	}

	virtual void MovePos(Point2F& pos, float len = 0.0)
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
	}

	void Move() { m_pos.x += m_cacheStep.x; m_pos.y += m_cacheStep.y;  }

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
};

class Shape : public Position
{
public:
	Shape(const Point2F& pos, float speed, int dir, UINT32 rgb, FLOAT alpha = 1.0F, LPARAM userdata = 0) : 
		Position(pos, speed, dir), m_pBrush(NULL), m_rgb(rgb), m_alpha(alpha), m_userdata(userdata), m_active(false)  {
	}

	~Shape() {
		D2DDiscardResources();
	}

	virtual void D2DOnCreateResources(IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* m_pIWICFactory, const D2DRectScaler* pRS) {
		UINT32 rgb =
			(m_rgb & 0x000000ff) << 16 |
			(m_rgb & 0x0000ff00) |
			(m_rgb & 0x00ff0000) >> 16;

		pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(rgb, m_alpha), &m_pBrush);
		m_active = true;
	}

	virtual void D2DDiscardResources() {
		SafeRelease(&m_pBrush);
	}

	void SetActive(bool b) { m_active = b;  }
	bool IsActive() { return m_active;  }

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

		return moveResult::ok;
	}

	ID2D1SolidColorBrush* GetBrush() { return m_pBrush; }

	virtual void Draw(ID2D1HwndRenderTarget* pRenderTarget, const D2DRectScaler* pRS = NULL) {}
	virtual void Draw(ID2D1HwndRenderTarget* pRenderTarget, Point2F pos, const D2DRectScaler* pRS = NULL) {}

	virtual void GetBoundingBox(RectF*, const Point2F& pos) const {}
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

	void SetColor(UINT32 rgb)	{ m_rgb = rgb;   }
	COLORREF GetColor()			{ return m_rgb;  }

	void SetAlpha(FLOAT alpha)	{ m_alpha = alpha; }
	FLOAT GetAlpha()			{ return m_alpha; }

	void SetUserData(LPARAM l) { m_userdata = l;  }
	LPARAM GetUserData() { return m_userdata;  }

protected:
	ID2D1SolidColorBrush* m_pBrush;
	UINT32 m_rgb;
	FLOAT m_alpha;
	LPARAM m_userdata;
	bool m_active;
};

class MovingCircle : public Shape
{
public:
	MovingCircle(const Point2F& pos, float radius, float speed, int dir, UINT32 rgb, FLOAT alpha = 1.0F, LPARAM userdata = 0) : Shape(pos, speed, dir, rgb, alpha, userdata), m_fRadius(radius) {}

	bool HitTest(Point2F pos) override {
		float distSq = (pos.x - m_pos.x) * (pos.x - m_pos.x) +
			(pos.y - m_pos.y) * (pos.y - m_pos.y);
		return distSq <= m_fRadius * m_fRadius;
	}

	void Draw(ID2D1HwndRenderTarget* pRenderTarget, const D2DRectScaler* pRS = NULL) override {
		D2D1_ELLIPSE e;
		e.point = GetPos();
		if (pRS)	pRS->Scale(&e.point);

		FLOAT r = m_fRadius;
		if (pRS)	pRS->ScaleNoOffset(&r);
		e.radiusX = e.radiusY = r;

		pRenderTarget->FillEllipse(&e, GetBrush());
	}

	void GetBoundingBox(RectF* p, const Point2F& pos) const {
		p->left = pos.x - m_fRadius;
		p->right = pos.x + m_fRadius;
		p->top = pos.y - m_fRadius;
		p->bottom = pos.y + m_fRadius;
	}

	bool BounceOffRectSides(const Shape& shape) {
		RectF r;
		shape.GetBoundingBox(&r, GetPos());

		Point2F pos = GetPos();	// where we are
		MovePos(pos);	// where we will be

		// Check we're anywhere near it first
		if ((pos.x + m_fRadius < r.left) || (pos.x - m_fRadius > r.right) ||
			(pos.y + m_fRadius < r.top) || (pos.y - m_fRadius > r.bottom)) {
			return false;
		}

		float endlen = sqrt(m_cacheStep.lengthsq());
		float len = 0.0;
		while (len < endlen) {	// Check in radius/2 steps
			len += 1.0;

			pos = GetPos();		// where we are
			MovePos(pos, len);	// where we will be

			// Check left and right bounce zone
			if (WillHitRectX(r, pos)) {
				SetPos(pos);
				BounceX();
				return true;
			}

			// Check top and bottom bounce zone
			if (WillHitRectY(r, pos)) {
				SetPos(pos);
				BounceY();
				return true;
			}
		}

		return false;
	}

	bool BounceOffRectCorners(const Shape& shape) {
		RectF r;
		shape.GetBoundingBox(&r, GetPos());

		Point2F pos = GetPos();	// where we are
		MovePos(pos);	// where we will be

		// Check we're anywhere near it first
		if ((pos.x + m_fRadius < r.left) || (pos.x - m_fRadius > r.right) ||
			(pos.y + m_fRadius < r.top) || (pos.y - m_fRadius > r.bottom)) {
			return false;
		}

		double radsq = m_fRadius * m_fRadius;
		std::vector<Point2F> corners;
		r.GetCorners(corners);

		float endlensq = m_cacheStep.lengthsq();
		float lensq = 0.0;
		while (lensq < endlensq) {	// Check in radius/2 steps
			lensq += 1.0;
			MovePos(pos, lensq);	// where we will be

			// Check all 4 corners
			for (auto& corner : corners) {
				if (pos.DistanceToSq(corner) < radsq - 0.1F) {
					SetPos(pos);
					BounceOffPoint(corner);
					return true;
				}
			}
		}

		return false;
	}

protected:
	void BounceOffPoint(const Point2F& pt) {
		// Gradient between pt and direction
		double direction = m_cacheStep.anglerad();
		double touch = pt.angleradTo(m_pos);
		double deflection = M_PI - (direction - touch) * 2;
		m_direction += deflection;
		UpdateCache();
	}

	bool WillHitRectX(const RectF& r, const Point2F& pos) {
		if ((pos.y >= r.top) && (pos.y <= r.bottom)) {
			if ((pos.x >= r.right) && (pos.x - m_fRadius <= r.right)) {
				return true;
			}
			if ((pos.x <= r.left) && (pos.x + m_fRadius >= r.left)) {
				return true;
			}
		}
		return false;
	}

	bool WillHitRectY(const RectF& r, const Point2F& pos) {
		if ((pos.x >= r.left) && (pos.x <= r.right)) {
			if ((pos.y >= r.bottom) && (pos.y - m_fRadius <= r.bottom)) {
				return true;
			}
			if ((pos.y <= r.top) && (pos.y + m_fRadius >= r.top)) {
				return true;
			}
		}
		return false;
	}

protected:
	FLOAT m_fRadius;
};

class MovingRectangle : public Shape
{
public:
	MovingRectangle(const Point2F& pos, float width, float height, float speed, int dir, UINT32 rgb, FLOAT alpha = 1.0F, LPARAM userdata = 0) :
		Shape(pos, speed, dir, rgb, alpha, userdata), m_fWidth(width), m_fHeight(height)
	{}

	void SetSize(FLOAT width, FLOAT height) {
		m_fWidth = width;
		m_fHeight = height;
	}

	FLOAT GetWidth() { return m_fWidth;  }
	FLOAT GetHeight() { return m_fHeight; }

	void SetWidth(FLOAT f) { m_fWidth = f; }
	void SetHeight(FLOAT f) { m_fHeight = f; }

	void Draw(ID2D1HwndRenderTarget* pRenderTarget, Point2F pos, const D2DRectScaler* pRS = NULL) override {
		FLOAT fWidth = m_fWidth;
		FLOAT fHeight = m_fHeight;
		if (pRS) {
			pRS->Scale(&pos);
			pRS->ScaleNoOffset(&fWidth);
			pRS->ScaleNoOffset(&fHeight);
		}

		D2D1_RECT_F r;
		r.left = pos.x;
		r.right = pos.x + fWidth;
		r.top = pos.y;
		r.bottom = pos.y + fHeight;
		pRenderTarget->FillRectangle(&r, GetBrush());
	}

	void Draw(ID2D1HwndRenderTarget* pRenderTarget, const D2DRectScaler* pRS = NULL) override {
		Draw(pRenderTarget, GetPos(), pRS);
	}

	void GetBoundingBox(RectF* p, const Point2F& pos) const {
		p->left = pos.x;
		p->right = pos.x + m_fWidth - 1;
		p->top = pos.y;
		p->bottom = pos.y + m_fHeight - 1;
	}

protected:
	FLOAT m_fWidth;
	FLOAT m_fHeight;
};

class MovingBitmap : public Shape
{
public:
	MovingBitmap(d2dBitmap* bitmap, const Point2F& pos, float width, float height, float speed, int dir, UINT32 rgb, FLOAT alpha = 1.0F, LPARAM userdata = 0) :
		m_bitmap(bitmap), Shape(pos, speed, dir, rgb, alpha, userdata), m_fWidth(width), m_fHeight(height) {}

	void SetBitmap(d2dBitmap* bitmap) {
		m_bitmap = bitmap;
	}

	void Draw(ID2D1HwndRenderTarget* pRenderTarget, const D2DRectScaler* pRS = NULL) override {
		Point2F pos = GetPos();
		FLOAT fWidth = m_fWidth;
		FLOAT fHeight = m_fHeight;
		if (pRS) {
			pRS->Scale(&pos);
			pRS->ScaleNoOffset(&fWidth);
			pRS->ScaleNoOffset(&fHeight);
		}

		D2D1_RECT_F r;
		r.left = pos.x;
		r.right = pos.x + fWidth;
		r.top = pos.y;
		r.bottom = pos.y + fHeight;
		m_bitmap->Render(pRenderTarget, r);
	}

	void GetBoundingBox(RectF* p, const Point2F& pos) const {
		p->left = pos.x;
		p->right = pos.x + m_fWidth;
		p->top = pos.y;
		p->bottom = pos.y + m_fHeight;
	}

protected:
	FLOAT m_fWidth;
	FLOAT m_fHeight;
	d2dBitmap* m_bitmap;
};

class MovingText : public Shape {
public:
	MovingText(LPCWSTR wsz, const Point2F& pos, float width, float height, float speed, int dir, DWRITE_TEXT_ALIGNMENT ta, UINT32 rgb, FLOAT alpha = 1.0F, LPARAM userdata = 0) :
		m_text(wsz),
		Shape(pos, speed, dir, rgb, alpha, userdata),
		m_fWidth(width), m_fHeight(height),
		m_pWTF(NULL),
		m_ta(ta)
	{
	}

	void D2DOnCreateResources(IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, const D2DRectScaler* pRS) override {
		FLOAT fHeight = m_fHeight;
		pRS->ScaleNoOffset(&fHeight);
		pDWriteFactory->CreateTextFormat(
			L"Arial",
			NULL,
			DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			fHeight,
			L"en-us",
			&m_pWTF
		);

		m_pWTF->SetTextAlignment(m_ta);

		__super::D2DOnCreateResources(pDWriteFactory, pRenderTarget, pIWICFactory, pRS);
	}

	void D2DDiscardResources() override {
		SafeRelease(&m_pWTF);
		__super::D2DDiscardResources();
	}

	void SetText(LPCWSTR wsz) {
		m_text = wsz;
	}

	void Draw(ID2D1HwndRenderTarget* pRenderTarget, const D2DRectScaler* pRS = NULL) override {
		Point2F pos = GetPos();
		FLOAT fWidth = m_fWidth;
		FLOAT fHeight = m_fHeight;
		if (pRS) {
			pRS->Scale(&pos);
			pRS->ScaleNoOffset(&fWidth);
			pRS->ScaleNoOffset(&fHeight);
		}

		D2D1_RECT_F r;
		r.left = pos.x;
		r.right = pos.x + fWidth;
		r.top = pos.y;
		r.bottom = pos.y + fHeight;
		pRenderTarget->DrawTextW(m_text.c_str(), (UINT32)m_text.length(), m_pWTF, r, m_pBrush);
	}

	virtual void GetBoundingBox(RectF* pRect, const Point2F& pos) const {
		pRect->left = pos.x;
		pRect->top = pos.y;
		pRect->right = pos.x + m_fWidth;
		pRect->bottom = pos.y + m_fHeight;
	}

protected:
	FLOAT m_fWidth;
	FLOAT m_fHeight;
	IDWriteTextFormat* m_pWTF;
	std::wstring m_text;
	DWRITE_TEXT_ALIGNMENT m_ta;
};

class MovingGroup : public Shape {
public:
	MovingGroup(const Point2F& pos, float speed, int dir, LPARAM userdata = 0) : Shape(pos, speed, dir, RGB(0, 0, 0), 1.0F, userdata) {
	}

	void Add(Shape* p) { m_members.push_back(p); }

	const std::vector<Shape*>& GetMembers() { return m_members;  }

	void D2DOnCreateResources(IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, const D2DRectScaler* pRS) override {
		for (auto m : m_members) {
			m->D2DOnCreateResources(pDWriteFactory, pRenderTarget, pIWICFactory, pRS);
		}
	}

	void D2DDiscardResources() override {
		for (auto m : m_members) {
			m->D2DDiscardResources();
		}
	}

	void Draw(ID2D1HwndRenderTarget* pRenderTarget, const D2DRectScaler* pRS = NULL) override {
		for (auto m : m_members) {
			Point2F pos = GetPos();
			pos += m->GetPos();
			m->Draw(pRenderTarget, pos, pRS);
		}
	}

	moveResult WillHitBounds(const w32Size& size) {
		for (auto m : m_members) {
			Point2F pos = GetPos();
			MovePos(pos);
			pos += m->GetPos();
			auto result = m->WillHitBounds(size, pos);
			if (result != Position::moveResult::ok)
				return result;
		}

		return Position::moveResult::ok;
	}

protected:
	std::vector<Shape*> m_members;
};

class TickDelta {
public:
	TickDelta(ULONGLONG periodMS, bool active = true) : m_periodMS(periodMS), m_active(active) {
		m_ullLast = GetTickCount64();
	}

	bool Elapsed(ULONGLONG tick) {
		if (!m_active)
			return false;

		if (tick - m_ullLast >= m_periodMS) {
			m_ullLast = tick;
			return true;
		}
		return false;
	}

	void SetActive(bool b) {
		m_ullLast = GetTickCount64();
		m_active = b;
	}

	void AddTicks(int ticks) {
		if ((ticks < 0) && (m_periodMS < -ticks)) {	// can't go < 0
			m_periodMS = 0;
		}
		else {
			m_periodMS += ticks;
		}
	}

	ULONGLONG Remaining(ULONGLONG tick) { return m_periodMS - (tick - m_ullLast); }

protected:
	ULONGLONG m_ullLast;
	ULONGLONG m_periodMS;
	bool m_active;
};

class D2DWorld
{
public:
	D2DWorld() : m_colorBackground(D2D1::ColorF::Black) {
	}

	~D2DWorld() {
	}

 	virtual bool D2DCreateResources(IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, D2DRectScaler* pRS)
	{
		return true;
	}

	virtual bool D2DDiscardResources() {
		for (auto p : m_shapes) {
			p->D2DDiscardResources();
		}
		m_shapes.clear();

		return true;
	}

	void AddShape(Shape* p, IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, D2DRectScaler* pRS, bool active = true) {
		p->D2DOnCreateResources(pDWriteFactory, pRenderTarget, pIWICFactory, pRS);
		p->SetActive(active);
		m_shapes.push_back(p);
	}

	void RemoveShape(Shape* p, bool del = true) {
		for (auto it = m_shapes.begin(); it != m_shapes.end(); ++it) {
			if (*it == p) {
				p->D2DDiscardResources();
				m_shapes.erase(it);
				if (del)
					delete p;
				return;
			}
		}
	}

	std::vector<Shape*>::iterator RemoveShape(std::vector<Shape*>::iterator it, bool del = true) {
		Shape* p = (*it);
		p->D2DDiscardResources();
		auto ret = m_shapes.erase(it);
		if (del)
			delete p;
		return ret;
	}

	void D2DPreRender(IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, D2DRectScaler* pRS) {
		// Now is the time to add queued shapes to the engine.
		for (auto p : m_shapesQueue) {
			AddShape(p.first, pDWriteFactory, pRenderTarget, pIWICFactory, pRS, p.second);
		}
		m_shapesQueue.clear();
	}

	void QueueShape(Shape* p, bool active = true) {
		m_shapesQueue.push_back(std::make_pair(p, active));
	}

	virtual bool Init() {
		return true;
	}

	virtual void DeInit() {
	}

	virtual bool D2DUpdate(ULONGLONG tick, const Point2F& ptMouse, std::queue<WindowEvent>& events) {
		for (auto p : m_shapes)
			if (p->IsActive())
				p->Move();
		return true;
	}

	virtual bool D2DRender(ID2D1HwndRenderTarget* pRenderTarget, D2DRectScaler* pRsFAR) {
		for (auto p : m_shapes)
			if (p->IsActive())
				p->Draw(pRenderTarget, pRsFAR);

		return true;
	}

	virtual w32Size D2DGetScreenSize() {
		return w32Size(1920, 1080);
	}

protected:
	int KeyDown(int keycode) {
		return ::GetAsyncKeyState(keycode) & 0x8000;
	}

	int KeyPressed(int keycode) {
		return ::GetAsyncKeyState(keycode) & 0x0001;
	}

protected:
	std::vector<Shape*> m_shapes;
	std::vector<std::pair<Shape*, bool>> m_shapesQueue;

public:
	D2D1::ColorF m_colorBackground;
};