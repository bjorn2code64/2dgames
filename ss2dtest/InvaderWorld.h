#pragma once

#include <SS2DWorld.h>

class InvaderWorld : public SS2DWorld
{
protected:
	const COLORREF m_textColour = RGB(0, 255, 0);
	const FLOAT m_textHeight = 18.0f;

	// dimensions
	const int m_screenWidth = 1000;
	const int m_screenHeight = 1080;
	const FLOAT m_scoreY = m_textHeight * 3;
	const FLOAT m_shipY = 50.0f;

	const FLOAT m_bulletWidth = 5.0f;
	const FLOAT m_bulletHeight = 30.0f;
	const COLORREF m_bulletColour = RGB(255, 255, 255);

	const FLOAT m_playerWidth = 80.0f;
	const FLOAT m_playerHeight = 30.0f;
	const Point2F m_playerStart = w32Point(0, 1000);
	const FLOAT m_playerSpeed = 10.0f;
	const FLOAT m_playerbulletSpeed = 20.0f;
	const int m_playerResetTime = 3000;
	const COLORREF m_playerColour = RGB(255, 255, 255);

	const int m_playerLives = 3;
	const Point2F m_playerLivesStart = Point2F(20.0F, 1050.0f);
	const FLOAT m_playerIndicatorSize = 0.6F;

	const FLOAT m_barrierWidth = 100.0f;
	const FLOAT m_barrierHeight = 50.0f;
	const int m_barrierCount = 4;
	const FLOAT m_barrierY = 900.0f;
	const COLORREF m_barrierColour = RGB(0, 255, 0);
	const int m_barrierDividerX = 8;
	const int m_barrierDividerY = 6;

	const FLOAT m_invaderWidth = 60.0f;
	const FLOAT m_invaderHeight = 40.0f;
	const FLOAT m_invaderBorder = 10.0f;
	const FLOAT m_invaderbulletSpeed = 15.0f;
	const DWORD m_invadersBulletChanceStart = 80;	// 1 in xxx
	const int m_invaderCols = 10;
	const int m_invaderRows = 5;
	const int m_invaderMoveDelayStart = 1000;	// in ms
	const FLOAT m_invaderSpeed = 25.0f;		// per move delay
	const COLORREF m_invaderColour = RGB(0, 255, 0);
	const int score_invader_hit = 30;

	const FLOAT m_shipWidth = 70.0f;
	const FLOAT m_shipHeight = 20.0f;
	const FLOAT m_shipSpeed = 4.0f;
	const int m_shipSpawnTime = 10000;
	const COLORREF m_shipColour = RGB(255, 0, 255);
	const int score_ship_hit = 100;

	const COLORREF m_gameOverColour = RGB(255, 255, 255);

	const enum class invaderType {
		octopus,
		crab,
		squid,
		hit
	};

public:
	InvaderWorld(Notifier& notifier) :
		m_notifier(notifier),

		// tickers
		m_tdInvaderMove(m_invaderMoveDelayStart),
		m_tdPlayerReset(m_playerResetTime, false),
		m_tdShip(m_shipSpawnTime),
		m_tdShipScore(2000, false),
		m_tdHitBitmap(100, false)
	{
		SS2DSetScreenSize(w32Size(1000, 1080));
	}

	~InvaderWorld() {
	}

	bool SS2DInit() {

		// Load bitmaps
		m_bitmapOctopus[0] = NewResourceBitmap(L"octopusClosed.png");
		m_bitmapOctopus[1] = NewResourceBitmap(L"octopusOpen.png");
		m_bitmapCrab[0] = NewResourceBitmap(L"crabClosed.png");
		m_bitmapCrab[1] = NewResourceBitmap(L"crabOpen.png");
		m_bitmapSquid[0] = NewResourceBitmap(L"squidClosed.png");
		m_bitmapSquid[1] = NewResourceBitmap(L"squidOpen.png");
		m_bitmapUFO = NewResourceBitmap(L"UFO.png");
		m_bitmapHit = NewResourceBitmap(L"invaderGone.png");

		// Create texts
		m_textScore = NewMovingText(L"", 0.0f, m_textHeight * 1.5f, 200.0f, m_textHeight, 0.0f, 0, DWRITE_TEXT_ALIGNMENT_CENTER, m_textColour);
		m_textScoreLabel = NewMovingText(L"Score", 0.0f, m_textHeight * 0.5f, 200.0f, m_textHeight, 0.0f, 0, DWRITE_TEXT_ALIGNMENT_CENTER, m_textColour);
		m_textShipScore = NewMovingText(L"100", 0.0f, 0.0f, m_shipWidth, m_textHeight, 0.0f, 0, DWRITE_TEXT_ALIGNMENT_CENTER, m_shipColour, 1.0, 0, false);
		m_textGameOver = NewMovingText(L"Game Over", 0.0f, m_screenHeight / 2.0F, (FLOAT)m_screenWidth, (FLOAT)m_screenHeight / 10.0F, 0.0f, 0, DWRITE_TEXT_ALIGNMENT_CENTER, m_gameOverColour, 1.0, 0, false);

		// Initialise Game Variables
		m_score = 0;
		m_livesRemaining = m_playerLives;
		m_player = NewMovingRectangle(m_playerStart.x, m_playerStart.y, m_playerWidth, m_playerHeight, 0, 0, m_playerColour);
		m_playerBullet = NewMovingRectangle(0, 0, m_bulletWidth, m_bulletHeight, 0, 0, m_bulletColour);

		m_frame = 0;
		m_invadersBulletChance = m_invadersBulletChanceStart;

		// Create Game Objects
		m_ship = NewMovingBitmap(m_bitmapUFO, 0, 0, m_shipWidth, m_shipHeight, m_shipSpeed, 90, 1.0F, 0, false);
		CreateInvaders();
		CreateBarriers();
		CreateLifeIndicators();

		// Update the score/lives on the screen
		UpdateIndicators();
		AddScore(0);

		return true;
	}

	bool SS2DUpdate(ULONGLONG tick, const Point2F& mouse, std::queue<WindowEvent>& events) override {
		UpdateCheckPlayerKeys();

		UpdateMoveInvaders(tick);

		UpdateMovePlayer();

		UpdateMovePlayerBullet();

		UpdateMoveInvaderBullets();

		// Game over? Don't do anything else.
		if (m_livesRemaining == 0) {
			return true;
		}

		UpdateFireInvaderBullets();

		if (m_ship) {
			UpdateShip(tick);
		}

		// Does the player need to respawn?
		if (m_tdPlayerReset.Elapsed(tick)) {	// respawn?
			UpdateIndicators();
			m_player->SetPos(m_playerStart);
			m_player->SetActive(true);
			m_playerBullet->SetActive(true);
			m_tdPlayerReset.SetActive(false);
		}

		while (!events.empty()) {
			auto& ev = events.front();
			if (ev.m_msg == WM_KEYDOWN) {
				if (ev.m_wParam == VK_ESCAPE) {
					// quit back to menu
					m_notifier.Notify(m_amQuit);
				}
			}
			events.pop();
		}

		if (m_tdHitBitmap.Elapsed(tick)) {
			m_hitBitmap->SetActive(false);
			m_tdHitBitmap.SetActive(false);
		}

		return true;
	}

protected:
	void CreateInvaders() {
		m_hitBitmap = NewMovingBitmap(m_bitmapHit, 0, 0, m_invaderWidth, m_invaderHeight, 0, 0, 1.0F, (LPARAM)invaderType::hit, false);

		m_groupInvaders = NewMovingGroup(0, m_scoreY + m_shipY, m_invaderSpeed, 90);

		for (FLOAT x = 0; x < m_invaderCols; x++) {
			for (int y = 0; y < m_invaderRows; y++) {
				invaderType invType = invaderType::squid;
				SS2DBitmap* p = m_bitmapSquid[m_frame];
				if ((y == 1) || (y == 2)) {
					p = m_bitmapCrab[m_frame];
					invType = invaderType::crab;
				}
				else if ((y == 3) || (y == 4)) {
					p = m_bitmapOctopus[m_frame];
					invType = invaderType::octopus;
				}

				m_groupInvaders->NewMovingBitmap(p,
					x * (m_invaderWidth + m_invaderBorder), y * (m_invaderHeight + m_invaderBorder),
					m_invaderWidth, m_invaderHeight,
					0, 0, 1.0f, (LPARAM)invType
				);
			}
		}

	}

	void CreateBarriers() {
		// Create the barriers as grids of destructable rectangles
		FLOAT step = (FLOAT)m_screenWidth / (FLOAT)m_barrierCount;
		for (int i = 0; i < m_barrierCount; i++) {
			FLOAT divBarrierWidth = m_barrierWidth / m_barrierDividerX;
			FLOAT divBarrierHeight = m_barrierHeight / m_barrierDividerY;

			m_groupBarriers[i] = NewMovingGroup(step / 2.0f + step * i - m_barrierWidth / 2, m_barrierY, 0, 0);
			for (int x = 0; x < m_barrierDividerX; x++) {
				for (int y = 0; y < m_barrierDividerY; y++) {
					m_groupBarriers[i]->NewMovingRectangle(
						x * divBarrierWidth, y * divBarrierHeight,
						divBarrierWidth + 1.0F, divBarrierHeight + 1.0F,
						0.0F, 0, m_barrierColour);
				}
			}
		}
	}

	void CreateLifeIndicators() {
		// Create the player life indicators
		Point2F playerLifePt = m_playerLivesStart;
		for (int i = 0; i < m_playerLives - 1; i++) {
			FLOAT piHeight = m_playerHeight * m_playerIndicatorSize;
			FLOAT piWidth = m_playerWidth * m_playerIndicatorSize;
			auto life = NewMovingRectangle(playerLifePt.x, playerLifePt.y, piWidth, piHeight, 0, 0, m_playerColour);
			auto bullet = NewMovingRectangle(0, 0, m_bulletWidth * m_playerIndicatorSize, m_bulletHeight * m_playerIndicatorSize, 0, 0, m_bulletColour);

			auto pos = life->GetPos();
			pos += Point2F((piWidth - m_bulletWidth) / 2.0f, -piHeight / 2.0f);
			bullet->SetPos(pos);

			playerLifePt.x += piWidth + 10.0F;
			m_playerLifeIndicators.push_back(life);
			m_playerLifeIndicators.push_back(bullet);
		}
	}

	void UpdateCheckPlayerKeys() {
		// Left/right?
		if (KeyDown(VK_LEFT)) {
			m_player->SetDirectionInDeg(-90);
			m_player->SetSpeed(m_playerSpeed);
		}
		else if (KeyDown(VK_RIGHT)) {
			m_player->SetDirectionInDeg(90);
			m_player->SetSpeed(m_playerSpeed);
		}
		else {
			m_player->SetSpeed(0.0f);
		}

		// Fire?
		if (KeyPressed(VK_LCONTROL)) {
			if (!m_playerBullet->GetUserData()) {
				// Create a player bullet
				m_playerBullet->SetDirectionInDeg(0);
				m_playerBullet->SetSpeed(m_playerbulletSpeed);
				m_playerBullet->SetUserData(1);
			}
		}
	}

	void UpdateIndicators() {
		int lastIndicator = (m_livesRemaining - 1) * 2;
		for (int i = 0; i < m_playerLifeIndicators.size(); i++) {
			if (i < lastIndicator) {
				m_playerLifeIndicators[i]->SetActive(true);
			}
			else {
				m_playerLifeIndicators[i]->SetActive(false);
			}
		}
	}

	void UpdateMoveInvaders(ULONGLONG tick) {
		if (m_tdInvaderMove.Elapsed(tick)) {
			// Time for an invader move
			// Replace the bitmap
			for (auto* p : m_groupInvaders->GetChildren()) {
				if (p->GetUserData() == (LPARAM)invaderType::squid) {
					((MovingBitmap*)p)->SetBitmap(m_bitmapSquid[m_frame]);
				}
				else if (p->GetUserData() == (LPARAM)invaderType::crab) {
					((MovingBitmap*)p)->SetBitmap(m_bitmapCrab[m_frame]);
				}
				else if (p->GetUserData() == (LPARAM)invaderType::octopus) {
					((MovingBitmap*)p)->SetBitmap(m_bitmapOctopus[m_frame]);
				}
			}
			m_frame = m_frame ? 0 : 1;

			// Check if they move, will any of them hit the end.
			if (m_groupInvaders->WillHitBounds(SS2DGetScreenSize()) != Shape::moveResult::ok) {
				// Move invaders down and send them back the other way
				m_groupInvaders->BounceX();
				m_groupInvaders->OffsetPos(Point2F(0.0f, 60.0f));
			}
			else {
				m_groupInvaders->Move();
			}
		}
	}

	void UpdateMovePlayer() {
		// Move the player
		if (m_player->WillHitBounds(SS2DGetScreenSize()) == Shape::moveResult::ok) {
			m_player->Move();
		}
		else {
			m_player->BounceX();
		}
	}

	void UpdateMovePlayerBullet() {
		// Move the player bullet
		if (m_playerBullet->IsActive() && m_playerBullet->GetUserData()) {
			if (m_playerBullet->WillHitBounds(SS2DGetScreenSize()) != Shape::moveResult::ok) {
				m_playerBullet->SetUserData(0);
			}
			else {
				m_playerBullet->Move();

				// Did we hit a barrier
				for (auto& g : m_groupBarriers) {
					std::vector<Shape*> hits;
					if (g->HitTestShapes(m_playerBullet, hits)) {
						for (auto barrierHit : hits) {
							g->RemoveChild(barrierHit);
							delete barrierHit;
						}
						m_playerBullet->SetUserData(0);	// Reset the player bullet
					}
				}

				// Did we hit an invader?
				auto invHit = m_groupInvaders->HitTestShape(m_playerBullet);
				if (invHit) {
					m_hitBitmap->SetPos(invHit->GetPos(true));
					m_groupInvaders->RemoveChild(invHit, true);	// Delete the invader
					m_hitBitmap->SetActive(true);			// Show the hit bitmap
					m_tdHitBitmap.SetActive(true);		// Start the hit bitmap timer
					m_playerBullet->SetUserData(0);		// Reset the bullet
					AddScore(score_invader_hit);
					m_tdInvaderMove.AddTicks(-20);		// speed up the invaders a little
					m_invadersBulletChance -= 1;		// increase bullet chance
				}

				// Did we hit a ship at the top?
				if (m_ship && m_ship->IsActive() && m_ship->HitTestShape(m_playerBullet)) { 
					m_ship->SetActive(false);
					m_playerBullet->SetUserData(0);

					// Display the score and set off a timer
					m_textShipScore->SetPos(m_ship->GetPos());
					m_textShipScore->SetActive(true);
					m_tdShipScore.SetActive(true);

					AddScore(score_ship_hit);
				}
			}
		}

		// Show the bullet on top of the player if it's not in flight
		if (!m_playerBullet->GetUserData()) {
			auto pos = m_player->GetPos();
			pos += Point2F((m_playerWidth - m_bulletWidth) / 2.0f, -m_playerHeight / 2.0f);
			m_playerBullet->SetPos(pos);
		}
	}

	void UpdateMoveInvaderBullets() {
		// Move (and destroy) invader bullets
		for (auto it = m_invaderBullets.begin(); it != m_invaderBullets.end();) {
			if ((*it)->IsActive()) {
				if ((*it)->WillHitBounds(SS2DGetScreenSize()) != Shape::moveResult::ok) {
					// we're out of bounds
					RemoveShape(*it, true);
					it = m_invaderBullets.erase(it);	// remove the bullet
					continue;
				}

				(*it)->Move();

				// Did we hit a barrier?
				bool hitBarrier = false;
				for (auto& g : m_groupBarriers) {
					std::vector<Shape*> hits;
					if (g->HitTestShapes(*it, hits)) {
						for (auto barrierHit : hits) {
							g->RemoveChild(barrierHit);
							delete barrierHit;
						}
						hitBarrier = true;
					}
				}

				// If so, destroy the bullet
				if (hitBarrier) {
					RemoveShape(*it, true);
					it = m_invaderBullets.erase(it);	// remove the bullet
					continue;
				}

				// did we hit the player?
				if (m_player->IsActive() && (*it)->HitTestShape(m_player)) {
					m_livesRemaining--;
					if (m_livesRemaining == 0) {
						// Go back to start screen/menu	-->>> GAME OVER
						m_textGameOver->SetActive(true);
						m_tdPlayerReset.SetActive(false);
						m_tdInvaderMove.SetActive(false);
					}

					m_tdPlayerReset.SetActive(true);
					m_player->SetActive(false);
					m_playerBullet->SetActive(false);
					RemoveShape(*it, true);
					it = m_invaderBullets.erase(it);	// remove the bullet
					continue;
				}
			}
			++it;
		}
	}

	void UpdateFireInvaderBullets() {
		// For each active invader, fire a bullet randomly
		auto& invaders = m_groupInvaders->GetChildren();
		DWORD bulletChance = (DWORD)(m_invadersBulletChance * invaders.size());
		for (auto inv : invaders) {
			if (!w32rand(bulletChance)) {
				Point2F ptBullet = inv->GetPos();
				ptBullet.x += (m_invaderWidth - m_bulletWidth) / 2;

				MovingRectangle* bullet = NewMovingRectangle(
					ptBullet.x, ptBullet.y, m_bulletWidth, m_bulletHeight, m_invaderbulletSpeed, 180, m_bulletColour
				);

				m_invaderBullets.push_back(bullet);
			}
		}
	}

	void UpdateShip(ULONGLONG tick) {
		if (m_ship->IsActive()) {
			// Move the ship along the top
			if (m_ship->WillHitBounds(SS2DGetScreenSize()) == Shape::moveResult::ok) {
				m_ship->Move();
			}
			else {
				m_ship->SetActive(false);
			}
		}

		if (m_tdShipScore.Elapsed(tick)) {
			m_textShipScore->SetActive(false);
		}

		// Spawn a ship along the top?
		if (m_tdShip.Elapsed(tick)) {
			m_ship->SetPos(Point2F(0.0f, m_scoreY + (m_shipY - m_shipHeight) / 2.0f));
			m_ship->SetActive(true);
		}
	}

	void AddScore(int n) {
		m_score += n;
		wchar_t buff[32];
		_snwprintf_s(buff, 32, L"%06d", m_score);
		m_textScore->SetText(buff);
	}

protected:
	MovingRectangle* m_player;
	MovingRectangle* m_playerBullet;
	std::vector<MovingRectangle*> m_playerLifeIndicators;

	MovingGroup* m_groupInvaders;
	std::vector<MovingRectangle*> m_invaderBullets;

	MovingGroup* m_groupBarriers[4];

	MovingBitmap* m_ship;
	MovingBitmap* m_hitBitmap;

	TickDelta m_tdInvaderMove;
	TickDelta m_tdPlayerReset;
	TickDelta m_tdShip;
	TickDelta m_tdShipScore;
	TickDelta m_tdHitBitmap;

	MovingText* m_textScore;
	MovingText* m_textScoreLabel;
	MovingText* m_textShipScore;
	MovingText* m_textGameOver;

	SS2DBitmap* m_bitmapOctopus[2];
	SS2DBitmap* m_bitmapCrab[2];
	SS2DBitmap* m_bitmapSquid[2];
	int m_frame;	// Invader bitmap frame switcher
	SS2DBitmap* m_bitmapHit;
	SS2DBitmap* m_bitmapUFO;

	// Changing stuff
	DWORD m_invadersBulletChance;
	int m_score;
	int m_livesRemaining;

	Notifier& m_notifier;

public:
	AppMessage m_amQuit;
};