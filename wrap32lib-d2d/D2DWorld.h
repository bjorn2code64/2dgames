#pragma once

#include "MovingShapes.h"

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

	void InitShape(Shape* p, IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, D2DRectScaler* pRS) {
		p->D2DOnCreateResources(pDWriteFactory, pRenderTarget, pIWICFactory, pRS);
		for (auto c : p->GetChildren()) {
			InitShape(c, pDWriteFactory, pRenderTarget, pIWICFactory, pRS);
		}
	}

	void AddShape(Shape* p, IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, D2DRectScaler* pRS, bool active = true) {
		InitShape(p, pDWriteFactory, pRenderTarget, pIWICFactory, pRS);
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