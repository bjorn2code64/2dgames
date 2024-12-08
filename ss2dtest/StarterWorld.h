#pragma once

#include <SS2DWorld.h>

class StarterWorld : public SS2DWorld
{
public:
	StarterWorld(Notifier& notifier) :
		m_notifier(notifier)
	{
	}

	bool SS2DInit() override {
		m_bitmap = NewResourceBitmap(L"octopusClosed.png");
		m_invader = NewMovingBitmap(m_bitmap, 100, 100, 60, 40, 20, 45);
		return true;
	}

	void SS2DDeInit() override {
		RemoveShape(m_invader, true);
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


		// Check for quit (Escape)
		while (!events.empty()) {
			auto& ev = events.front();
			if (ev.m_msg == WM_KEYDOWN) {
				if (ev.m_wParam == VK_ESCAPE) {
					m_notifier.Notify(m_amQuit);	// QUIT - Go back to menu
				}
			}

			events.pop();
		}

		return __super::SS2DUpdate(tick, ptMouse, events);
	}

protected:
	MovingBitmap* m_invader;
	SS2DBitmap* m_bitmap;
	Notifier& m_notifier;

public:
	AppMessage m_amQuit;
};
