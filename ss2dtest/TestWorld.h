#pragma once

#include <SS2DWorld.h>
#include <list>

class TestWorld : public SS2DWorld
{
	const FLOAT m_playerMoveSpeed = 10;
	const int m_screenWidth = 900;
	const int m_screenHeight = 900;

	const std::vector<COLORREF> c_colorsAvailable = {
		RGB(255, 0, 0),
		RGB(0, 255, 0),
		RGB(0, 0, 255),
		RGB(255, 255, 0),
		RGB(255, 0, 255),
		RGB(0, 255, 255),
		RGB(255, 255, 255)
	};

public:
	TestWorld(Notifier& notifier) : 
		m_notifier(notifier)
	{
		SS2DSetScreenSize(w32Size(m_screenWidth, m_screenHeight));
	}

	bool SS2DInit() override {
		for (auto c : c_colorsAvailable) {
			m_brushes.push_back(NewResourceBrush(c));
		}

		m_player = NewMovingRectangle(430, 850, 100, 30, 0, 0, m_brushWhite);
		m_playerBullet = NewMovingCircle(430, 850, 15, 0, 0, m_brushes[w32rand(6)]);

		for (int i = 0; i < 30; i++) {
			m_circles.push_back(NewMovingCircle(i * 30.0f, 500, 15, 2, 90, m_brushes[w32rand(6)]));
		}

		for (int i = 0; i < 30; i++) {
			m_circles.push_back(NewMovingCircle(i * 30.0f, 300, 15, 2, 90, m_brushes[w32rand(6)]));
		}
		return true;
	}

	void SS2DDeInit() override {
	}

	MovingCircle* HitTestCircles(MovingCircle* bullet) {
		for (auto c : m_circles) {
			if (bullet->HitTestShape(c)) {
				return c;
			}
		}
		return NULL;
	}

	bool SS2DUpdate(ULONGLONG tick, const Point2F& ptMouse, std::queue<WindowEvent>& events) override {
		// Wrap any circles gone off the board
		for (auto c : m_circles) {
			if (c->WillHitBounds(SS2DGetScreenSize()) == Shape::moveResult::hitboundsright) {
				c->OffsetPos(Point2F((FLOAT)-m_screenWidth, 0));
			}
		}

		// Bullet off top of screen?
		if (m_playerBullet->WillHitBounds(SS2DGetScreenSize()) == Shape::moveResult::hitboundstop) {
			m_playerBullet->SetSpeed(0);
		}

		for (auto it = m_bullets.begin(); it != m_bullets.end(); ) {
			auto bullet = *it;
			auto circlehit = HitTestCircles(bullet);
			if (circlehit) {
				circlehit->SetSpeed(bullet->GetSpeed() / 2);
				circlehit->SetDirectionInDeg(0);
				Point2F posHit = circlehit->GetPos();
				circlehit->OffsetPos(Point2F(0, -30));
				m_bullets.push_front(circlehit);

				auto it2 = std::find(m_circles.begin(), m_circles.end(), circlehit);
				if (it2 != m_circles.end()) {
					m_circles.erase(it2);
				}

				bullet->SetSpeed(0);
				bullet->SetPos(posHit);
				bullet->SetSpeed(2);
				bullet->SetDirectionInDeg(90);
				m_circles.push_back(bullet);
				it = m_bullets.erase(it);
				if (bullet == m_playerBullet) {
					m_playerBullet = NewMovingCircle(430, 850, 15, 0, 0, m_brushes[w32rand(6)]);
				}
			}
			else
				++it;
		}

		// Move the player based on keys down
		if (KeyDown(VK_RIGHT) && KeyDown(VK_LEFT)) {
			m_player->SetSpeed(0);
		}
		else if (KeyDown(VK_RIGHT)) {
			m_player->SetSpeed(m_playerMoveSpeed);
			m_player->SetDirectionInDeg(90);
		}
		else if (KeyDown(VK_LEFT)) {
			m_player->SetSpeed(m_playerMoveSpeed);
			m_player->SetDirectionInDeg(270);
		}
		else {
			m_player->SetSpeed(0);
		}

		// If player bullet isn't moving, track the player
		if (m_playerBullet->GetSpeed() == 0) {
			auto pos = m_player->GetPos();
			pos.x += 50;
			m_playerBullet->SetPos(pos);
		}

		// Check for quit (Escape)
		while (!events.empty()) {
			auto& ev = events.front();
			if (ev.m_msg == WM_KEYDOWN) {
				if (ev.m_wParam == VK_ESCAPE) {
					m_notifier.Notify(m_amQuit);	// QUIT
				}
				else if (ev.m_wParam == VK_CONTROL) {	// fire
					m_playerBullet->SetSpeed(20);
					m_playerBullet->SetDirectionInDeg(0);
					m_bullets.push_back(m_playerBullet);
				}
			}

			events.pop();
		}

		return __super::SS2DUpdate(tick, ptMouse, events);
	}

protected:
	Notifier& m_notifier;

	std::vector<SS2DBrush*> m_brushes;
	MovingRectangle* m_player;
	MovingCircle* m_playerBullet;
	std::list<MovingCircle*> m_circles;
	std::list<MovingCircle*> m_bullets;

public:
	AppMessage m_amQuit;
};
