#pragma once

#include <SS2DWorld.h>

class StarterWorld : public SS2DWorld
{
public:
	bool SS2DInit() override {
		m_bitmap = NewResourceBitmap(L"octopusClosed.png");
		m_invader = NewMovingBitmap(m_bitmap, 0, 0, 60, 40, 20, 45);
		return true;
	}

	bool SS2DUpdate(ULONGLONG tick, const Point2F& ptMouse, std::queue<WindowEvent>& events) override {
		// Game logic here

		switch (m_invader->WillHitBounds(SS2DGetScreenSize())) {
		case Shape::moveResult::hitboundsbottom:
		case Shape::moveResult::hitboundstop:
			m_invader->BounceY();
			break;
		case Shape::moveResult::hitboundsleft:
		case Shape::moveResult::hitboundsright:
			m_invader->BounceX();
			break;
		}

		return __super::SS2DUpdate(tick, ptMouse, events);
	}

protected:
	MovingBitmap* m_invader;
	SS2DBitmap* m_bitmap;
};
