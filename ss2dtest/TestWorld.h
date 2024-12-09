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

		m_player = NewMovingRectangle(430, 850, 100, 30, 0, 0, GetDefaultBrush());
		m_playerBullet = NewMovingCircle(430, 850, 15, 0, 0, m_brushes[w32rand(6)]);

		for (int i = 0; i < 31; i++) {
			auto c = NewMovingCircle(i * 30.0f - 30.0f, 700, 15, 1, 90, m_brushes[w32rand(6)]);
			m_circles.push_back(c);
			m_lines[0].push_back(c);
		}

		for (int i = 0; i < 31; i++) {
			auto c = NewMovingCircle(i * 30.0f, 500, 15, 1, 270, m_brushes[w32rand(6)]);
			m_circles.push_back(c);
			m_lines[1].push_back(c);
		}

		for (int i = 0; i < 31; i++) {
			auto c = NewMovingCircle(i * 30.0f, 200, 15, 1, 270, m_brushes[w32rand(6)]);
			m_circles.push_back(c);
			m_lines[2].push_back(c);
		}

		for (int i = 0; i < 31; i++) {
			auto c = NewMovingCircle(i * 30.0f, 100, 15, 1, 270, m_brushes[w32rand(6)]);
			m_circles.push_back(c);
			m_lines[3].push_back(c);
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

	void CheckLine(std::list<MovingCircle*>& line) {
		COLORREF crLast = 0;
		int match = 0;
		std::list<MovingCircle*>::iterator itStart;
		for (int i = 0; i < 2; i++) {
			for (auto it = line.begin(); it != line.end(); ++it) {
				if (crLast == (*it)->GetBrush()->GetColor()) {
					match++;
				}
				else {
					if (match > 3) {
						// zap the lot
						while (itStart != it) {
							RemoveShape(*itStart);
							itStart = line.erase(itStart);
						}
					}
					match = 1;
					itStart = it;
					crLast = (*it)->GetBrush()->GetColor();
				}
			}
		}
	}

	void CheckLines() {
		for (int i = 0; i < 4; i++) {
			CheckLine(m_lines[i]);
		}
	}

	bool SS2DUpdate(ULONGLONG tick, const Point2F& ptMouse, std::queue<WindowEvent>& events) override {
		// Wrap any circles gone off the board
		RectF hit(-30.0f, 0, SS2DGetScreenSize().cx + 60.0f, SS2DGetScreenSize().cy);
		for (auto c : m_circles) {
			auto hitResult = c->WillHitBounds(hit);

			if (hitResult == Shape::moveResult::hitboundsright) {
				c->OffsetPos(Point2F((FLOAT)-m_screenWidth - 30, 0));
			}
			else if (hitResult == Shape::moveResult::hitboundsleft) {
				c->OffsetPos(Point2F((FLOAT)m_screenWidth + 30, 0));
			}
		}

		// Bullet off top of screen?
		if (m_playerBullet->WillHitBounds(SS2DGetScreenSize()) == Shape::moveResult::hitboundstop) {
			m_playerBullet->SetSpeed(0);
		}

		auto circlehit = HitTestCircles(m_playerBullet);
		if (circlehit) {
			// swap colours with the bullet
			auto bbr = m_playerBullet->GetBrush();
//				auto bPos = bullet->GetPos();
			auto cbr = circlehit->GetBrush();
			auto cPos = circlehit->GetPos();
			m_playerBullet->SetPos(Point2F(cPos.x, cPos.y - 31));

			m_playerBullet->SetBrush(cbr);
			circlehit->SetBrush(bbr);
			m_playerBullet->SetSpeed(m_playerBullet->GetSpeed() / 2);
			CheckLines();
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
	std::list<MovingCircle*> m_lines[4];

public:
	AppMessage m_amQuit;
};
