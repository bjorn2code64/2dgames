#pragma once

#include "MovingShapes.h"
#include "SS2DBrush.h"

class TickDelta {
public:
	TickDelta(ULONGLONG periodMS = 1000, bool active = true) : m_periodMS(periodMS), m_active(active) {
		m_ullLast = GetTickCount64();
	}

	void SetPeriod(ULONGLONG periodMS, bool active = true) {
		m_periodMS = periodMS;
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
		m_active = b;
		m_ullLast = GetTickCount64();
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

class SS2DWorld
{
public:
	SS2DWorld() :
		m_colorBackground(D2D1::ColorF::Black),
		m_screenSize(1920, 1080)
	{
		m_brushWhite = NewResourceBrush(RGB(255, 255, 255));
	}

	~SS2DWorld() {
	}

	virtual bool SS2DDiscardResources() {
		for (auto p : m_shapes) {
			p->SS2DDiscardResources();
		}
		m_shapes.clear();

		for (auto b : m_brushes) {
			b->Clear();
		}
		m_brushes.clear();
		return true;
	}

	SS2DBrush* NewResourceBrush(COLORREF cr, FLOAT alpha = 1.0) {
		SS2DBrush* p = new SS2DBrush(cr, alpha);
		QueueResourceBrush(p);
		return p;
	}

	SS2DBitmap* NewResourceBitmap(LPCWSTR filePath) {
		SS2DBitmap* p = new SS2DBitmap(filePath);
		QueueResourceBitmap(p);
		return p;
	}

	MovingRectangle* NewMovingRectangle(FLOAT x, FLOAT y, FLOAT width, FLOAT height, FLOAT speed, int dir, SS2DBrush* brush, FLOAT alpha = 1.0F, LPARAM userdata = 0, bool active = true) {
		MovingRectangle* p = new MovingRectangle(x, y, width, height, speed, dir, brush, alpha, userdata);
		QueueShape(p, active);
		return p;
	}

	MovingCircle* NewMovingCircle(FLOAT x, FLOAT y, FLOAT radius, FLOAT speed, int dir, SS2DBrush* brush, FLOAT alpha = 1.0F, LPARAM userdata = 0, bool active = true) {
		MovingCircle* p = new MovingCircle(x, y, radius, speed, dir, brush, alpha, userdata);
		QueueShape(p, active);
		return p;
	}

	MovingBitmap* NewMovingBitmap(SS2DBitmap* bitmap, FLOAT x, FLOAT y, FLOAT width, FLOAT height, FLOAT speed, int dir, FLOAT alpha = 1.0F, LPARAM userdata = 0, bool active = true) {
		MovingBitmap* p = new MovingBitmap(bitmap, x, y, width, height, speed, dir, alpha, userdata);
		QueueShape(p, active);
		return p;
	}

	MovingText* NewMovingText(LPCWSTR wsz, FLOAT x, FLOAT y, FLOAT width, FLOAT height, FLOAT speed, int dir, DWRITE_TEXT_ALIGNMENT ta, SS2DBrush* brush, FLOAT alpha = 1.0F, LPARAM userdata = 0, bool active = true) {
		MovingText* p = new MovingText(wsz, x, y, width, height, speed, dir, ta, brush, alpha, userdata);
		QueueShape(p, active);
		return p;
	}

	MovingGroup* NewMovingGroup(FLOAT x, FLOAT y, FLOAT speed, int dir, LPARAM userdata = 0, bool active = true) {
		MovingGroup* p = new MovingGroup(x, y, speed, dir, userdata);
		QueueShape(p, active);
		return p;
	}

	void InitShape(Shape* p, IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, D2DRectScaler* pRS) {
		p->SS2DCreateResources(pDWriteFactory, pRenderTarget, pIWICFactory, pRS);
		for (auto c : p->GetChildren()) {
			InitShape(c, pDWriteFactory, pRenderTarget, pIWICFactory, pRS);
		}
	}

	void AddShape(Shape* p, IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, D2DRectScaler* pRS, bool active = true) {
		InitShape(p, pDWriteFactory, pRenderTarget, pIWICFactory, pRS);
		p->SetActive(active);
		m_shapes.push_back(p);
	}

	void RemoveAllShapes() {
		for (auto it = m_shapes.begin(); it != m_shapes.end(); ++it) {
			(*it)->SS2DDiscardResources();
		}
		m_shapes.clear();
	}

	void RemoveShape(Shape* p, bool del = false) {
		for (auto it = m_shapes.begin(); it != m_shapes.end(); ++it) {
			if (*it == p) {
				p->SS2DDiscardResources();
				m_shapes.erase(it);
				if (del) {
					delete p;
				}
				return;
			}
		}
	}

	std::vector<Shape*>::iterator RemoveShape(std::vector<Shape*>::iterator it) {
		Shape* p = (*it);
		p->SS2DDiscardResources();
		return m_shapes.erase(it);
	}

	void D2DPreRender(IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, D2DRectScaler* pRS) {
		while (!m_brushQueue.empty()) {
			auto p = m_brushQueue.front();
			p->Create(pRenderTarget);
			m_brushes.push_back(p);
			m_brushQueue.pop();
		}

		while (!m_bitmapQueue.empty()) {
			auto p = m_bitmapQueue.front();
			p->LoadFromFile(pRenderTarget, pIWICFactory);
			m_bitmapQueue.pop();
		}

		// Now is the time to add queued shapes to the engine.
		for (auto p : m_shapesQueue) {
			AddShape(p.first, pDWriteFactory, pRenderTarget, pIWICFactory, pRS, p.second);
		}
		m_shapesQueue.clear();
	}

	void QueueShape(Shape* p, bool active = true) {
		m_shapesQueue.push_back(std::make_pair(p, active));
	}

	void QueueResourceBrush(SS2DBrush* p) {
		m_brushQueue.push(p);
	}

	void QueueResourceBitmap(SS2DBitmap* p) {
		m_bitmapQueue.push(p);
	}

	virtual bool SS2DInit() {
		return true;
	}

	void DeInit() {
		SS2DDeInit();
		for (auto c : m_shapes) {
			delete c;
		}
	}

	virtual void SS2DDeInit() {
	}

	virtual bool SS2DUpdate(ULONGLONG tick, const Point2F& ptMouse, std::queue<WindowEvent>& events) {
		for (auto p : m_shapes)
			if (p->IsActive())
				p->Move();
		return true;
	}

	virtual bool D2DRender(ID2D1HwndRenderTarget* pRenderTarget, DWORD dwFlags, D2DRectScaler* pRsFAR) {
		for (auto p : m_shapes)
			if (p->IsActive())
				p->Draw(pRenderTarget, dwFlags, pRsFAR);

		return true;
	}

	virtual w32Size& SS2DGetScreenSize() {
		return m_screenSize;
	}

	void SS2DSetScreenSize(const w32Size& size) {
		m_screenSize = size;
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
	std::vector<SS2DBrush*> m_brushes;
	std::vector<std::pair<Shape*, bool>> m_shapesQueue;
	std::queue<SS2DBitmap*> m_bitmapQueue;
	std::queue<SS2DBrush*> m_brushQueue;
	w32Size m_screenSize;
	SS2DBrush* m_brushWhite;

public:
	D2D1::ColorF m_colorBackground;
};