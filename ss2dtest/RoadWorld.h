#pragma once

#include <SS2DWorld.h>

class RoadWorld : public SS2DWorld
{
	const int m_screenWidth = 1920;
	const int m_screenHeight = 1080;

public:
	RoadWorld(Notifier& notifier) : 
		m_notifier(notifier),
		m_player(910, 900, 50, 100, 0, 0, RGB(255, 255, 255)),
		m_roadLeft(800),
		m_roadWidth(400),
		m_rdNewRoad(200),
		m_speed(5)
	{
	}

	w32Size SS2DGetScreenSize() override {
		return w32Size(m_screenWidth, m_screenHeight);
	}

	bool SS2DInit() override {
		m_player.SetPos(Point2F(910, 900));
		QueueShape(&m_player);
		return true;
	}

	void SS2DDeInit() override {
		for (auto r : m_road) {
			RemoveShape(r, true);
		}
		RemoveShape(&m_player);
		m_road.clear();
	}

	bool SS2DUpdate(ULONGLONG tick, const Point2F& ptMouse, std::queue<WindowEvent>& events) override {
		// Delete road that goes off the bottom
		for (auto it = m_road.begin(); it != m_road.end(); ) {
			auto r = *it;
			if (r->WillHitBounds(SS2DGetScreenSize()) == Shape::moveResult::hitboundsbottom) {
				RemoveShape(r, true);
				it = m_road.erase(it);
			}
			else {
				++it;
			}
		}

		// Move the player based on keys down
		if (KeyDown(VK_RIGHT) && KeyDown(VK_LEFT)) {
			m_player.SetSpeed(0);
		}
		else if (KeyDown(VK_RIGHT)) {
			m_player.SetSpeed(5);
			m_player.SetDirectionInDeg(90);
		}
		else if (KeyDown(VK_LEFT)) {
			m_player.SetSpeed(5);
			m_player.SetDirectionInDeg(270);
		}
		else {
			m_player.SetSpeed(0);
		}

		// Did the player hit a road
		for (auto r : m_road) {
			if (r->HitTestShape(&m_player)) {
				m_notifier.Notify(m_amQuit);	// QUIT
			}
		}

		// Every timer tick, create new bit of road at the top
		if (m_rdNewRoad.Elapsed(tick)) {
			// Create new bit of road
			MovingRectangle* left = new MovingRectangle(m_roadLeft, 0, 50, 50, m_speed, 180, RGB(255, 255, 255));
			MovingRectangle* right = new MovingRectangle(m_roadLeft + m_roadWidth, 0, 50, 50, m_speed, 180, RGB(255, 255, 255));
			QueueShape(left);
			QueueShape(right);
			m_road.push_back(left);
			m_road.push_back(right);

			m_roadLeft += w32rand(-50, 50);
			m_speed += 0.08f;
			m_rdNewRoad.AddTicks(-1);
			for (auto r : m_road) {
				r->SetSpeed(m_speed);
			}
		}

		// Check for quit (Escape)
		while (!events.empty()) {
			auto& ev = events.front();
			if (ev.m_msg == WM_KEYDOWN) {
				if (ev.m_wParam == VK_ESCAPE) {
					m_notifier.Notify(m_amQuit);	// QUIT
				}
			}

			events.pop();
		}

		return __super::SS2DUpdate(tick, ptMouse, events);
	}

protected:
	MovingRectangle m_player;
	Notifier& m_notifier;

	FLOAT m_roadLeft;
	FLOAT m_roadWidth;
	FLOAT m_speed;

	std::list<MovingRectangle*> m_road;

	TickDelta m_rdNewRoad;

public:
	AppMessage m_amQuit;
};
