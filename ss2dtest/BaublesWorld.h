#pragma once

#include <SS2DWorld.h>
#include <list>

class BaublesWorld : public SS2DWorld
{
	const FLOAT c_playerMoveSpeed = 10;
	const int c_screenWidth = 900;
	const int c_screenHeight = 900;
	const FLOAT c_ballDiameter = 56.0f;
	const FLOAT c_ballRadius = c_ballDiameter / 2.0f;
	const int c_baublesPerRow = (int)(c_screenWidth / c_ballDiameter + 1);
	const FLOAT m_playerWidth = 100.0f;
	const FLOAT m_playerHeight = 30.0f;

public:
	BaublesWorld(Notifier& notifier) : 
		m_notifier(notifier)
	{
		SS2DSetScreenSize(w32Size(c_screenWidth, c_screenHeight));
	}

	MovingGroup* NewBauble(FLOAT x, FLOAT y, int dir) {
		int col = w32rand(4);
		auto g = NewMovingGroup(x, y, 1, dir, (LPARAM)col);
		g->NewMovingBitmap(m_baubleBitmaps[col], 0, 0, c_ballDiameter, c_ballDiameter, 0, 0);	// picture at 0
		g->NewMovingCircle(c_ballRadius, c_ballRadius, c_ballRadius, 0, 0, m_brushInvisible);		// mask at 1
		return g;
	}

	bool SS2DInit() override {
		// Create the resources we need
		m_brushInvisible = NewResourceBrush(RGB(255, 255, 255), 0.0f);

		m_baubleBitmaps.push_back(NewResourceBitmap(L"bauble_red.png"));
		m_baubleBitmaps.push_back(NewResourceBitmap(L"snowman1.png"));
		m_baubleBitmaps.push_back(NewResourceBitmap(L"bauble_yellow.png"));
		m_baubleBitmaps.push_back(NewResourceBitmap(L"bauble_green.png"));
		m_baubleBitmaps.push_back(NewResourceBitmap(L"bauble_orange.png"));

		m_baubleMovingBitmaps.push_back(NULL);
		m_baubleMovingBitmaps.push_back(NewResourceBitmap(L"snowman2.png"));
		m_baubleMovingBitmaps.push_back(NULL);
		m_baubleMovingBitmaps.push_back(NULL);
		m_baubleMovingBitmaps.push_back(NULL);

		m_scoreLabel = NewMovingText(L"Score", 100, 10, 100, 20, 0, 0, DWRITE_TEXT_ALIGNMENT_CENTER, GetDefaultBrush());
		m_scoreValue = NewMovingText(L"00000", 200, 10, 100, 20, 0, 0, DWRITE_TEXT_ALIGNMENT_CENTER, GetDefaultBrush());
		m_score = 0;

		// Create the shapes we need
		m_player = NewMovingRectangle(430, 850, m_playerWidth, m_playerHeight, 0, 0, GetDefaultBrush());

		int col = w32rand(4);
		m_bullet = NewMovingGroup(430, 850, 0, 0, (LPARAM)col);
		m_bullet->NewMovingBitmap(m_baubleBitmaps[col], 0, 0, c_ballDiameter, c_ballDiameter, 0, 0);	// picture at 0
		m_bullet->NewMovingCircle(c_ballRadius, c_ballRadius, c_ballRadius, 0, 0, m_brushInvisible);		// mask at 1

		for (int i = 0; i < c_baublesPerRow; i++) { 
			auto g = NewBauble(i * c_ballDiameter, 700, 90);
			m_baubles.push_back(g);
			m_lines[0].push_back(g);
		}

		for (int i = 0; i < c_baublesPerRow; i++) {
			auto g = NewBauble(i * c_ballDiameter, 500, 270);
			m_baubles.push_back(g);
			m_lines[1].push_back(g);
		}

		for (int i = 0; i < c_baublesPerRow; i++) {
			auto g = NewBauble(i * c_ballDiameter, 300, 90);
			m_baubles.push_back(g);
			m_lines[2].push_back(g);
		}

		for (int i = 0; i < c_baublesPerRow; i++) {
			auto g = NewBauble(i * c_ballDiameter, 100, 270);
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
		std::list<MovingGroup*>::iterator itStart = line.begin();
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

							RemoveShape(sf);				// Remove it as a standalone from the engine
							group->AddChildAndOffset(sf);	// Add it to the group (which is already in the engine)
							m_snowSettled.push(sf);
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
		RectF hit(-c_ballDiameter, 0, SS2DGetScreenSize().cx + (2 * c_ballDiameter), (FLOAT)SS2DGetScreenSize().cy);
		for (auto c : m_baubles) {
			auto hitResult = c->WillHitBounds(hit);

			if (hitResult == Shape::moveResult::hitboundsright) {
				c->OffsetPos(Point2F((FLOAT)-c_screenWidth - c_ballDiameter, 0));
			}
			else if (hitResult == Shape::moveResult::hitboundsleft) {
				c->OffsetPos(Point2F((FLOAT)c_screenWidth + c_ballDiameter, 0));
			}
		}

		// Bullet off top of screen?
		if (m_bullet->WillHitBounds(SS2DGetScreenSize()) == Shape::moveResult::hitboundstop) {
			m_bullet->SetSpeed(0);
			int col = w32rand(4);;
			m_bullet->SetUserData(col);
			((MovingBitmap*)m_bullet->GetChildren()[0])->SetBitmap(m_baubleBitmaps[col]);
		}

		// Did the bullet hit anything
		auto baublehit = HitTestBaubles(m_bullet);
		if (baublehit) {
			// swap bitmaps with the bullet
			auto bmp = m_bullet->GetUserData();
			auto cbr = baublehit->GetUserData();
			auto cPos = baublehit->GetPos();
			m_bullet->SetPos(Point2F(cPos.x, cPos.y - c_ballDiameter - 1));

			m_bullet->SetUserData(cbr);
			SS2DBitmap* bmpNext = m_baubleMovingBitmaps[cbr] ? m_baubleMovingBitmaps[cbr] : m_baubleBitmaps[cbr];
			((MovingBitmap*)m_bullet->GetChildren()[0])->SetBitmap(bmpNext);

			baublehit->SetUserData(bmp);
			((MovingBitmap*)baublehit->GetChildren()[0])->SetBitmap(m_baubleBitmaps[bmp]);

			m_bullet->SetSpeed(m_bullet->GetSpeed() / 2);
			CheckLines();
		}

		// Move the player based on keys down
		if (KeyDown(VK_RIGHT) && KeyDown(VK_LEFT)) {
			m_player->SetSpeed(0);
		}
		else if (KeyDown(VK_RIGHT)) {
			m_player->SetSpeed(c_playerMoveSpeed);
			m_player->SetDirectionInDeg(90);
		}
		else if (KeyDown(VK_LEFT)) {
			m_player->SetSpeed(c_playerMoveSpeed);
			m_player->SetDirectionInDeg(270);
		}
		else {
			m_player->SetSpeed(0);
		}

		// If player bullet isn't moving, track the player
		if (m_bullet->GetSpeed() == 0) {
			auto pos = m_player->GetPos();
			m_player->MovePos(pos);
			pos.x += m_playerWidth / 2 - c_ballDiameter / 2;
			pos.y += m_playerHeight - c_ballDiameter;
			m_bullet->SetPos(pos);
		}

		// Check for quit (Escape)
		while (!events.empty()) {
			auto& ev = events.front();
			if (ev.m_msg == WM_KEYDOWN) {
				if (ev.m_wParam == VK_ESCAPE) {
					m_notifier.Notify(m_amQuit);	// QUIT
				}
				else if (ev.m_wParam == VK_CONTROL) {	// fire
					m_bullet->SetSpeed(20);
					m_bullet->SetDirectionInDeg(0);
				}
			}

			events.pop();
		}

		// Make SNOW!
		int x = w32rand(c_screenWidth - 1);
		int dir = w32rand(160, 200);
		int speed = w32rand(50, 150);

		auto snowflake = NewMovingCircle((FLOAT)x, 2, 1, (FLOAT)speed / 100.0f, dir, GetDefaultBrush());
		m_snow.push_back(snowflake);

		// Clean up SNOW!
		if (m_snowSettled.size() > 1000) {
			auto sf = m_snowSettled.front();
			m_snowSettled.pop();
			auto group = (MovingGroup*)sf->GetParent();
			group->RemoveChild(sf, true);
		}

		return __super::SS2DUpdate(tick, ptMouse, events);
	}

protected:
	Notifier& m_notifier;

	SS2DBrush* m_brushInvisible;
	MovingRectangle* m_player;
	MovingGroup* m_bullet;
	std::list<MovingGroup*> m_baubles;
	std::list<MovingGroup*> m_lines[4];

	std::vector<SS2DBitmap*> m_baubleBitmaps;
	std::vector<SS2DBitmap*> m_baubleMovingBitmaps;

	std::vector<MovingCircle*> m_snow;
	std::queue<MovingCircle*> m_snowSettled;

	MovingText* m_scoreLabel;
	MovingText* m_scoreValue;
	int m_score;

public:
	AppMessage m_amQuit;
};
