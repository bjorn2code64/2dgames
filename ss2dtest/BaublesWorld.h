#pragma once

#include <SS2DWorld.h>
#include <list>

class BaublesWorld : public SS2DWorld
{
	const FLOAT m_playerMoveSpeed = 10;
	const int m_screenWidth = 900;
	const int m_screenHeight = 900;
	const FLOAT m_ballDiameter = 56.0f;
	const FLOAT m_ballRadius = m_ballDiameter / 2.0f;
	const int m_ballCount = (int)(m_screenWidth / m_ballDiameter + 1);
	const FLOAT m_playerWidth = 100.0f;
	const FLOAT m_playerHeight = 30.0f;

public:
	BaublesWorld(Notifier& notifier) : 
		m_notifier(notifier)
	{
		SS2DSetScreenSize(w32Size(m_screenWidth, m_screenHeight));
	}

	MovingGroup* NewBauble(FLOAT x, FLOAT y, int dir) {
		int col = w32rand(4);
		auto g = NewMovingGroup(x, y, 1, dir, (LPARAM)col);
		g->NewMovingBitmap(m_baubleBitmaps[col], 0, 0, m_ballDiameter, m_ballDiameter, 0, 0);	// picture at 0
		g->NewMovingCircle(m_ballRadius, m_ballRadius, m_ballRadius, 0, 0, m_brushMask);		// mask at 1
		return g;
	}

	bool SS2DInit() override {
		// Create the resources we need
		m_brushMask = NewResourceBrush(RGB(255, 255, 255), 0.0f);
		m_baubleBitmaps.push_back(NewResourceBitmap(L"bauble_red.png"));	// 56x56
		m_baubleBitmaps.push_back(NewResourceBitmap(L"bauble_blue.png"));	// 56x56
		m_baubleBitmaps.push_back(NewResourceBitmap(L"bauble_yellow.png"));	// 56x56
		m_baubleBitmaps.push_back(NewResourceBitmap(L"bauble_green.png"));	// 56x56
		m_baubleBitmaps.push_back(NewResourceBitmap(L"bauble_orange.png"));	// 56x56

		m_scoreLabel = NewMovingText(L"Score", 100, 10, 100, 20, 0, 0, DWRITE_TEXT_ALIGNMENT_CENTER, GetDefaultBrush());
		m_scoreValue = NewMovingText(L"00000", 200, 10, 100, 20, 0, 0, DWRITE_TEXT_ALIGNMENT_CENTER, GetDefaultBrush());
		m_score = 0;

		// Create the shapes we need
		m_player = NewMovingRectangle(430, 850, m_playerWidth, m_playerHeight, 0, 0, GetDefaultBrush());

		int col = w32rand(4);
		m_playerBullet = NewMovingGroup(430, 850, 0, 0, (LPARAM)col);
		m_playerBullet->NewMovingBitmap(m_baubleBitmaps[col], 0, 0, m_ballDiameter, m_ballDiameter, 0, 0);	// picture at 0
		m_playerBullet->NewMovingCircle(m_ballRadius, m_ballRadius, m_ballRadius, 0, 0, m_brushMask);		// mask at 1

		for (int i = 0; i < m_ballCount; i++) { 
			auto g = NewBauble(i * m_ballDiameter, 700, 90);
			m_baubles.push_back(g);
			m_lines[0].push_back(g);
		}

		for (int i = 0; i < m_ballCount; i++) {
			auto g = NewBauble(i * m_ballDiameter, 500, 270);
			m_baubles.push_back(g);
			m_lines[1].push_back(g);
		}

		for (int i = 0; i < m_ballCount; i++) {
			auto g = NewBauble(i * m_ballDiameter, 300, 90);
			m_baubles.push_back(g);
			m_lines[2].push_back(g);
		}

		for (int i = 0; i < m_ballCount; i++) {
			auto g = NewBauble(i * m_ballDiameter, 100, 270);
			m_baubles.push_back(g);
			m_lines[3].push_back(g);
		}
		return true;
	}

	void SS2DDeInit() override {
		m_baubles.clear();
		for (int i = 0; i < 4; i++) {
			m_lines[i].clear();
		}
		m_snow.clear();
		m_baubleBitmaps.clear();
	}

	MovingGroup* HitTestBaubles(MovingGroup* bullet) {
		MovingCircle* bulletMask = (MovingCircle*)bullet->GetChildren()[1];
		for (auto group : m_baubles) {
			MovingCircle* mask = (MovingCircle*)group->GetChildren()[1];
			if (group->IsActive() && bulletMask->HitTestShape(mask)) {
				return group;
			}
		}
		return NULL;
	}

	void UpdateScore() {
		m_scoreValue->SetText(std::to_wstring(m_score).c_str());
	}

	void CheckLine(std::list<MovingGroup*>& line) {
		int crLast = 0;
		int match = 0;
		std::list<MovingGroup*>::iterator itStart;
		for (int i = 0; i < 2; i++) {
			for (auto it = line.begin(); it != line.end(); ++it) {
				auto nextBauble = *it;
				if (nextBauble->IsActive() && (crLast == nextBauble->GetUserData())) {
					match++;
				}
				else {
					if (match > 3) {
						// zap the lot
						while (itStart != it) {
							(*itStart)->SetActive(false);	// Set the whole group inactive
							if (++itStart == line.end()) {
								itStart = line.begin();
							}
							m_score += 10;
							UpdateScore();
						}
					}
					match = 1;
					itStart = it;
					crLast = (int)nextBauble->GetUserData();
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
		// Clean up snow that's fallen outside the screen border
		for (auto it = m_snow.begin(); it != m_snow.end(); ) {
			auto s = *it;
			if (s->WillHitBounds(SS2DGetScreenSize()) != Shape::moveResult::ok) {
				it = m_snow.erase(it);
				RemoveShape(s, true);
			}
			else {
				++it;
			}
		}

		// Hit test the snow with the baubles and settle if there's a hit.
		for (auto it = m_snow.begin(); it != m_snow.end(); ) {
			auto sf = *it;

			bool hit = false;
			for (auto group : m_baubles) {
				if (group->IsActive()) {
					if (w32rand(100) == 0) {
						auto mask = (MovingCircle*)group->GetChildren()[1];
						if (sf->HitTestShape(mask)) {
							// Stop the snowflake and move into the bauble group that it hit
							sf->SetSpeed(0);
							sf->OffsetPos(Point2F(-group->GetPos().x, -group->GetPos().y));
							RemoveShape(sf);	// remove it from the engine
							group->AddChild(sf);	// add it to the group
							hit = true;
							break;
						}
					}
				}
			}

			if (hit) {
				it = m_snow.erase(it);
			}
			else
				++it;
		}

		// Wrap any baubles gone off the board
		RectF hit(-m_ballDiameter, 0, SS2DGetScreenSize().cx + (2 * m_ballDiameter), (FLOAT)SS2DGetScreenSize().cy);
		for (auto c : m_baubles) {
			auto hitResult = c->WillHitBounds(hit);

			if (hitResult == Shape::moveResult::hitboundsright) {
				c->OffsetPos(Point2F((FLOAT)-m_screenWidth - m_ballDiameter, 0));
			}
			else if (hitResult == Shape::moveResult::hitboundsleft) {
				c->OffsetPos(Point2F((FLOAT)m_screenWidth + m_ballDiameter, 0));
			}
		}

		// Bullet off top of screen?
		if (m_playerBullet->WillHitBounds(SS2DGetScreenSize()) == Shape::moveResult::hitboundstop) {
			m_playerBullet->SetSpeed(0);
			int col = w32rand(4);;
			m_playerBullet->SetUserData(col);
			((MovingBitmap*)m_playerBullet->GetChildren()[0])->SetBitmap(m_baubleBitmaps[col]);
		}

		// Did the bullet hit anything
		auto circlehit = HitTestBaubles(m_playerBullet);
		if (circlehit) {
			// swap bitmaps with the bullet
			auto bmp = m_playerBullet->GetUserData();
			auto cbr = circlehit->GetUserData();
			auto cPos = circlehit->GetPos();
			m_playerBullet->SetPos(Point2F(cPos.x, cPos.y - m_ballDiameter - 1));

			m_playerBullet->SetUserData(cbr);
			((MovingBitmap*)m_playerBullet->GetChildren()[0])->SetBitmap(m_baubleBitmaps[cbr]);

			circlehit->SetUserData(bmp);
			((MovingBitmap*)circlehit->GetChildren()[0])->SetBitmap(m_baubleBitmaps[bmp]);

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
			pos.x += m_playerWidth / 2 - m_ballDiameter / 2;
			pos.y += m_playerHeight - m_ballDiameter;
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

		// Make SNOW!
		int x = w32rand(m_screenWidth - 1);
		int dir = w32rand(170, 190);
		int speed = w32rand(50, 150);

		auto snowflake = NewMovingCircle((FLOAT)x, 2, 1, (FLOAT)speed / 100.0f, dir, GetDefaultBrush());
		m_snow.push_back(snowflake);

		return __super::SS2DUpdate(tick, ptMouse, events);
	}

protected:
	Notifier& m_notifier;

	SS2DBrush* m_brushMask;
	MovingRectangle* m_player;
	MovingGroup* m_playerBullet;
	std::list<MovingGroup*> m_baubles;
	std::list<MovingGroup*> m_lines[4];

	std::vector<SS2DBitmap*> m_baubleBitmaps;

	std::vector<MovingCircle*> m_snow;

	MovingText* m_scoreLabel;
	MovingText* m_scoreValue;
	int m_score;

public:
	AppMessage m_amQuit;
};
