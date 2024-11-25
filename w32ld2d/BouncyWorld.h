#pragma once

#include <D2DWorld.h>

class BouncyWorld : public D2DWorld
{
public:
	const int m_screenWidth = 1920;
	const int m_screenHeight = 1080;

	const int m_numShapes = 1000;
	const float m_maxRadius = 60.0f;
	const float m_minRadius = 10.0f;

	~BouncyWorld() {
		for (auto p : m_shapes) {
			delete p;
		}
	}
	bool D2DCreateResources(IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, D2DRectScaler* pRS) override {
		for (int i = 0; i < m_numShapes; i++) {
			FLOAT speed = w32randf(5.0f, 15.f);
			DWORD direction = w32rand(0, 359);
			COLORREF color = RGB(w32rand(0, 256), w32rand(0, 256), w32rand(0, 256));
			if (w32rand(1)) {
				FLOAT width = w32randf(m_minRadius, m_maxRadius);
				FLOAT height = w32randf(m_minRadius, m_maxRadius);

				Point2F pos(w32randf(0, m_screenWidth - width), w32randf(0, m_screenHeight - height));

				AddShape(new MovingRectangle(
					pos,
					width, height,
					speed,
					direction,
					color),
					pDWriteFactory, pRenderTarget, pIWICFactory, pRS);
			}
			else {
				float radius = w32randf(m_minRadius, m_maxRadius);
				Point2F pos(w32randf(radius, m_screenWidth - radius), w32randf(radius, m_screenHeight - radius));

				AddShape(new MovingCircle(
					pos,
					radius,
					speed,
					direction,
					color),
					pDWriteFactory, pRenderTarget, pIWICFactory, pRS);
			}
		}

		return true;
	}

	bool D2DUpdate(ULONGLONG tick, const Point2F& ptMouse, std::queue<WindowEvent>& events) override {
		D2D1_RECT_U rectBounds;
		rectBounds.left = rectBounds.top = 0;
		rectBounds.right = m_screenWidth;
		rectBounds.bottom = m_screenHeight;

		auto it = m_shapes.begin();
		while (it != m_shapes.end()) {

			// Bounce the shape off the edge
			switch ((*it)->WillHitBounds(rectBounds)) {
			case Position::moveResult::hitboundsleft:
				(*it)->BounceX();
				break;
			case Position::moveResult::hitboundsright:
				(*it)->BounceX();
				break;
			case Position::moveResult::hitboundstop:
				(*it)->BounceY();
				break;
			case Position::moveResult::hitboundsbottom:
				(*it)->BounceY();
				break;
			}

			// Is the mouse touching it?
			if ((*it)->HitTest(ptMouse)) {
				it = RemoveShape(it);	// Delete it
			}
			else {
				++it;
			}
		}
		return __super::D2DUpdate(tick, ptMouse, events);
	}
};