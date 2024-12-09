#pragma once

#include <list>

#include <SS2DWorld.h>

FLOAT LimitF(FLOAT x, FLOAT min, FLOAT max) {
	return (x < min) ? min : (x > max ? max : x);
}

class BreakoutWorld : public SS2DWorld
{
protected:
	// Bat properties
	const FLOAT m_batStartWidth = 150.0f;
	const FLOAT m_batHeight = 20.0f;
	const FLOAT m_brickWidth = 96.0f;
	const FLOAT m_brickHeight = 30.0f;

	// Ball properties
	const FLOAT m_ballRadius = 10.0f;
	const FLOAT m_ballStartSpeed = 10.0f;
	const FLOAT m_ballSpeedupIncrement = 1.0f;

	// Bullet details for shooter mode
	const FLOAT m_bulletWidth = 10.0f;
	const FLOAT m_bulletHeight = 30.0f;
	const COLORREF m_bulletColour = RGB(255, 0, 0);
	const FLOAT m_playerbulletSpeed = 20.0f;

	// Brick identifiers (held in userdata)
	const int m_brickNormal = 0;
	const int m_brickBatLarger = 1;
	const int m_brickBallSlower = 2;
	const int m_brickMultiball = 3;
	const int m_brickShooter = 4;

	// Special timeouts
	const int batLargerTime = 10000;	// ms
	const int ballFasterTime = 4000;
	const int ballShooterTime = 10000;

public:
	BreakoutWorld(Notifier& notifier) :
		m_notifier(notifier)
	{
	}
	~BreakoutWorld() {}

	void GenerateBricks() {
		/*
		srand(time(NULL));

		for (FLOAT x = 0; x < m_screenWidth; x += m_brickWidth) {
					for (int y = 5; y < 35; y++) {
						double random = rand();
						if (random > RAND_MAX / 2) {
							continue;
						}

						MovingRectangle* pBrick = new MovingRectangle(Point2F(x, y * m_brickHeight),
							m_brickWidth, m_brickHeight, 0, 0,
							RGB(255, 0, 0)
						);

						m_bricks.push_back(pBrick);
						AddShape(pBrick, pDWriteFactory, pRenderTarget, pIWICFactory, pRS);
					}
				}*/

		MovingGroup* pCurrentGroup = NULL;
		File f;
		f.Open(L"level1.txt", GENERIC_READ, OPEN_EXISTING);
		std::string s;
		int y = 0;
		while (f.ReadLine(s)) {
			// Skip comment lines
			if ((s.length() > 0) && (s.at(0) == '#')) {
				continue;
			}

			int x = 0;
			for (int i = 0; i < s.length(); i++) {
				SS2DBrush* brush = NULL;
				int brickType = m_brickNormal;
				bool isBrick = true;
				SS2DBitmap* bitmap = NULL;

				switch (s.at(i)) {
					case '0':
						brickType = m_brickNormal;
						brush = m_brushBrick;
						break;
					case '1':
						brickType = m_brickBatLarger;
						bitmap = m_bitmapBatLarger;
						break;
					case '2':
						brickType = m_brickBallSlower;
						brush = m_brushBrickSlower;
						break;
					case '3':
						brickType = m_brickMultiball;
						bitmap = m_bitmapMultiball;
						break;
					case '4':
						brickType = m_brickShooter;
						bitmap = m_bitmapShooter;
						break;
					default:
						isBrick = false;
						break;
				}

				if (isBrick) {
					if (pCurrentGroup == NULL) {
						pCurrentGroup = NewMovingGroup(0, 0, 0, 0);
						m_groups.push_back(pCurrentGroup);
					}

					if (bitmap) {
						MovingBitmap* pBrick = NewMovingBitmap(bitmap, x * m_brickWidth, y * m_brickHeight,
							m_brickWidth, m_brickHeight, 0, 0
						);
						m_bricks.push_back(pBrick);
						pBrick->SetUserData(brickType);
						pCurrentGroup->AddChild(pBrick);
					}
					else if (brush) {
						MovingRectangle* pBrick = NewMovingRectangle(x * m_brickWidth, y * m_brickHeight,
							m_brickWidth, m_brickHeight, 0, 0,
							brush
						);
						m_bricks.push_back(pBrick);
						pBrick->SetUserData(brickType);
						pCurrentGroup->AddChild(pBrick);
					}
				}

				x++;
			}

			if (x == 0) {
				if (pCurrentGroup) {
					pCurrentGroup = NULL;
				}
			}

			y++;
		}

/*		for (FLOAT x = 0; x < m_screenWidth; x += m_brickWidth) {
			for (int y = 3; y < 19; y++) {
				if ((y == 7) || (y == 8) || (y == 13) || (y == 14))
					continue;

				MovingRectangle* pBrick = new MovingRectangle(Point2F(x, y * m_brickHeight),
					m_brickWidth, m_brickHeight, 0, 0,
					GetRowColor(y)
				);

				double random = rand();
				double step = RAND_MAX / 20;
				if (random < step) {
					pBrick->SetUserData(m_brickBatSmaller);
					pBrick->SetColor(RGB(0, 255, 0));
				}
				else if (random < 2 * step) {
					pBrick->SetUserData(m_brickBallFaster);
					pBrick->SetColor(RGB(0, 0, 255));
				}

				m_bricks.push_back(pBrick);
				AddShape(pBrick, pDWriteFactory, pRenderTarget, pIWICFactory, pRS);
			}
		}*/
	}

	bool SS2DInit() {
		m_brushBrick = NewResourceBrush(RGB(200, 200, 200));
		m_brushBrickSlower = NewResourceBrush(RGB(0, 0, 255));
		m_brushRed = NewResourceBrush(RGB(255, 0, 0));

		m_bitmapMultiball = NewResourceBitmap(L"multiball.png");
		m_bitmapShooter = NewResourceBitmap(L"shooter.png");
		m_bitmapBatLarger = NewResourceBitmap(L"batlarger.png");

		m_bat = NewMovingRectangle(0, 0, m_batStartWidth, m_batHeight, 0, 0, GetDefaultBrush());
		m_countdownShooter = NewMovingRectangle(400, 1030, 100.0F, 20.0f, 0.0f, 0, m_brushRed);
		m_playerBullet = NewMovingRectangle(0, 0, m_bulletWidth, m_bulletHeight, 0, 0, m_brushRed, 1.0, 0, false);

		m_textScoreLabel = NewMovingText(L"Score:", 10, 1030, 200.0f, 20.0f, 0, 0, DWRITE_TEXT_ALIGNMENT_CENTER, GetDefaultBrush());
		m_textScore = NewMovingText(L"0", 50, 1030, 200.0f, 20.0f, 0, 0, DWRITE_TEXT_ALIGNMENT_TRAILING, GetDefaultBrush());

		m_tdBatLarger.SetPeriod(batLargerTime, false);
		m_tdBallFaster.SetPeriod(ballFasterTime, false);
		m_tdShooterMode.SetPeriod(ballShooterTime, false);

		m_batWidthRequired = m_batStartWidth;
		m_ballSpeed = m_ballStartSpeed;
		m_score = 0;
		m_textScore->SetText(std::to_wstring(m_score).c_str());

		m_balls.push_back(NewMovingCircle(200.0f, 600.0f, m_ballRadius, m_ballStartSpeed, 135, GetDefaultBrush()));

		GenerateBricks();

		m_tdBallFaster.SetActive(true);

		return true;
	}

	void SS2DDeInit() {
		m_balls.clear();
		m_bricks.clear();
	}

	bool SS2DUpdate(ULONGLONG tick, const Point2F& mouse, std::queue<WindowEvent>& events) override {
		m_colorBackground = D2D1::ColorF::Black;

		// Larger bat timeout elapsed?
		if (m_tdBatLarger.Elapsed(tick)) {
			// Put the bat back to original size
			m_tdBatLarger.SetActive(false);
			m_batWidthRequired = m_batStartWidth;
		}

		if (m_tdBallFaster.Elapsed(tick)) {
			m_ballSpeed += m_ballSpeedupIncrement;
			for (auto pBall : m_balls) {
				pBall->SetSpeed(m_ballSpeed);
			}
		}

		// Shoot mode timer
		if (m_tdShooterMode.Elapsed(tick)) {
			m_shooterMode = false;
			m_tdShooterMode.SetActive(false);
			if (m_playerBullet->GetUserData() == 0) {	// hide the bullet if it's not inflight
				m_playerBullet->SetActive(false);
			}
			m_countdownShooter->SetActive(false);
		}

		while (!events.empty()) {
			auto& e = events.front();
			if (e.m_msg == WM_LBUTTONDOWN) {
				if (m_shooterMode) {
					if (m_playerBullet->GetUserData() == 0) {
						m_playerBullet->SetDirectionInRad(0);
						m_playerBullet->SetSpeed(m_playerbulletSpeed);
						m_playerBullet->SetUserData(1);
					}
				}
				m_countdownShooter->SetWidth((FLOAT)m_tdShooterMode.Remaining(tick) / 50.0f);
			}
			else if (e.m_msg == WM_KEYDOWN) {
				if (e.m_wParam == VK_ESCAPE) {
					m_notifier.Notify(m_amQuit);
				}
			}
			events.pop();
		}

		FLOAT batWidth = m_bat->GetWidth();

		// Move the bat to follow the mouse position
		// Position the bat such that the middle is level with the mouse x position
		FLOAT batX = mouse.x - batWidth / 2.0F;
		m_bat->SetPos(Point2F(LimitF(batX, 0.0f, SS2DGetScreenSize().cx - batWidth), 1000.0f));
		if (m_playerBullet->GetUserData() == 0) {
			FLOAT bulletX = mouse.x - m_bulletWidth / 2.0F;
			m_playerBullet->SetPos(Point2F(LimitF(bulletX, 0.0f, SS2DGetScreenSize().cx - m_bulletWidth), 1000.0f + m_batHeight - m_bulletHeight));
		}
		else {
			if (m_playerBullet->WillHitBounds(SS2DGetScreenSize()) != Shape::moveResult::ok) {
				ResetBullet();
			}
		}

		auto it = m_balls.begin();
		while (it != m_balls.end())
		{
			auto pBall = *it;
			if (CheckBallHitScreenEdges(pBall)) {	// true means it's off the bottom
				it = m_balls.erase(it);
				RemoveShape(pBall, true);
			}
			else {
				++it;
			}
		}

		for (auto pBall : m_balls) {
			WillBallHitBat(pBall);
		}

		for (auto pBall : m_balls) {
			CheckBallHitBrick(pBall);
		}

		CheckBatHitBrick();	// for falling bricks

		CheckBulletHitBrick();

		// Ensure the ball angle is away from the horizontal
		for (auto pBall : m_balls) {
			AdjustBallAngle(pBall);
		}

		// Is the bat the correct size?
		if (batWidth < m_batWidthRequired) {
			m_bat->SetWidth(batWidth + 5.0f);
		}
		else if (batWidth > m_batWidthRequired) {
			m_bat->SetWidth(batWidth - 5.0f);
		}

		// Move any falling bricks
		for (auto pBrick : m_bricks) {
			// Has this brick gone past the player's bat?
			if (pBrick->WillHitBounds(SS2DGetScreenSize()) == Shape::moveResult::hitboundsbottom) {
				pBrick->SetActive(false);
			}
		}

		return __super::SS2DUpdate(tick, mouse, events);
	}

	void BrickWasHit(Shape* pBrickHit) {
		// Start the brick falling if it's a special
		if ((int)pBrickHit->GetUserData() != m_brickNormal) {
			pBrickHit->SetDirectionInDeg(180);
			pBrickHit->SetSpeed(2.0F);
			MovingGroup* pGroup = (MovingGroup *)pBrickHit->GetParent();
			pGroup->RemoveChild(pBrickHit);
			QueueShape(pBrickHit);
		}
		else {
			pBrickHit->SetActive(false);
		}
		m_score += 10;
		m_textScore->SetText(std::to_wstring(m_score).c_str());
	}

	bool CheckBallHitScreenEdges(Shape* pBall) {
		if (!pBall->IsActive())
			return false;

		// Will the ball hit an edge?
		switch (pBall->WillHitBounds(SS2DGetScreenSize())) {
		case Shape::moveResult::hitboundsright:
		case Shape::moveResult::hitboundsleft:
			pBall->BounceX();
			break;
		case Shape::moveResult::hitboundstop:
			pBall->BounceY();
			break;
		case Shape::moveResult::hitboundsbottom:
			return true;
		}
		return false;
	}

	void WillBallHitBat(MovingCircle* pBall) {
		if (!pBall->IsActive())
			return;

		// Did the bat hit the ball?
		if (pBall->WillBounceOffRectSides(m_bat) || pBall->WillBounceOffRectCorners(m_bat)) {
			FLOAT batWidth = m_bat->GetWidth();
			FLOAT middleOfBat = m_bat->GetPos().x + batWidth / 2.0f;
			FLOAT middleOfBall = pBall->GetPos().x + m_ballRadius;
			FLOAT diff = middleOfBall - middleOfBat;	// +ve = ball to the right of the middle
			diff /= (batWidth / 2.0f);		// diff will go from -1 -> +1
			double deviation = diff * M_PI / 4.0;	// 45 deg
			pBall->AddDirection(deviation);
		}
	}

	void CheckBallHitBrick(MovingCircle* pBall) {
		int special = m_brickNormal;
		bool hit = false;
		Shape* pBrickHit = NULL;

		for (auto group : m_groups) {
			std::vector<Shape*> bricksHit;
			if (group->WillHitShapes(*pBall, bricksHit) > 0) {
				for (auto pBrick : bricksHit) {
					if (pBrick->IsActive() && (pBrick->GetSpeed() == 0.0F)) {	// brick is visible and not falling
						if (pBall->WillBounceOffRectSides(pBrick)) {
							special = (int)pBrick->GetUserData();
							hit = true;
							pBrickHit = pBrick;
							break;
						}
					}
				}

				if (hit) {
					for (auto pBrick : bricksHit) {
						if (pBrick->IsActive() && (pBrick->GetSpeed() == 0.0F)) {
							if (pBall->WillBounceOffRectCorners(pBrick)) {
								special = (int)pBrick->GetUserData();
								pBrickHit = pBrick;
								break;
							}
						}
					}
				}
			}
		}

		/*		// Has the ball hit a brick
		for (auto pBrick : m_bricks) {
			if (pBrick->IsActive() && (pBrick->GetSpeed() == 0.0F)) {
				if (pBall->WillBounceOffRectSides(*pBrick)) {
					special = (int)pBrick->GetUserData();
					hit = true;
					pBrickHit = pBrick;
					break;
				}
			}
		}

		if (!hit) {
			for (auto pBrick : m_bricks) {
				if (pBrick->IsActive() && (pBrick->GetSpeed() == 0.0F)) {
					if (pBall->WillBounceOffRectCorners(*pBrick)) {
						special = (int)pBrick->GetUserData();
						pBrickHit = pBrick;
						break;
					}
				}
			}
		}*/

		if (pBrickHit)
			BrickWasHit(pBrickHit);
	}

	void CheckBatHitBrick() {
		for (auto pBrick : m_bricks) {
			if (pBrick->IsActive() && m_bat->HitTestShape(pBrick)) {
				// The bat hit a (falling) brick. Activate the effect.
				int special = (int)pBrick->GetUserData();
				if (special == m_brickBatLarger) {
					m_batWidthRequired *= 2.0;
					if (m_batWidthRequired > 300.0F) {
						m_batWidthRequired = 300.0f;
					}
					m_tdBatLarger.SetActive(true);
				}
				else if (special == m_brickBallSlower) {
					m_ballSpeed -= 5.0f;
					if (m_ballSpeed < 5.0f) {
						m_ballSpeed = 5.0f;
					}
					for (auto pBall : m_balls) {
						pBall->SetSpeed(m_ballSpeed);
					}
				}
				else if (special == m_brickMultiball) {
					std::list<MovingCircle*> newBalls;	// a list of newly created balls

					for (auto pBall : m_balls) {
						// Get the pos/dir of each ball
						Point2F pos = pBall->GetPos();
						int direction = pBall->GetDirectionInDeg();

						// Add two more balls at this position
						MovingCircle* pBallNew = new MovingCircle(pos.x, pos.y, m_ballRadius, m_ballStartSpeed, direction + 2, GetDefaultBrush());
						newBalls.push_back(pBallNew);
						pBallNew = new MovingCircle(pos.x, pos.y, m_ballRadius, m_ballStartSpeed, direction - 2, GetDefaultBrush());
						newBalls.push_back(pBallNew);
					}

					// Add all the newly create balls into the world
					for (auto pBall : newBalls) {
						QueueShape(pBall);
						m_balls.push_back(pBall);
					}
				}
				else if (special == m_brickShooter) {
					m_shooterMode = true;
					m_tdShooterMode.SetActive(true);
					m_playerBullet->SetActive(true);
					m_countdownShooter->SetActive(true);
				}
				pBrick->SetActive(false);
			}
		}
	}

	void ResetBullet() {
		m_playerBullet->SetUserData(0);
		if (!m_shooterMode) {
			m_playerBullet->SetActive(false);
		}
	}

	void CheckBulletHitBrick() {
		if (!m_shooterMode) {
			return;
		}

		for (auto pBrick : m_bricks) {
			if (pBrick->IsActive() && (pBrick->GetSpeed() == 0.0F)) {
				if (m_playerBullet->HitTestShape(pBrick)) {
					BrickWasHit(pBrick);
					ResetBullet();
				}
			}
		}
	}

	void AdjustBallAngle(MovingCircle* pBall) {
		// The ball travelling too horizontally can be very boring while the player waits for it
		// to descend so if ever we see this, correct the angle to a steeper one.
		double direction = pBall->GetDirectionInDeg();
		if ((direction > 70) && (direction <= 90)) {
			pBall->SetDirectionInDeg(69);
		}
		else if ((direction < 110) && (direction >= 90)) {
			pBall->SetDirectionInDeg(110);
		}
		else if ((direction < -70) && (direction >= -90)) {
			pBall->SetDirectionInDeg(-70);
		}
		else if ((direction > -110) && (direction <= -90)) {
			pBall->SetDirectionInDeg(-110);
		}
	}

protected:
	Notifier& m_notifier;

	SS2DBrush* m_brushBrick;
	SS2DBrush* m_brushBrickSlower;
	SS2DBrush* m_brushRed;

	MovingRectangle* m_bat;
	std::list<MovingCircle*> m_balls;
	std::vector<Shape*> m_bricks;
	std::vector<MovingGroup*> m_groups;
	int m_score;

	FLOAT m_batWidthRequired;
	TickDelta m_tdBatLarger;

	TickDelta m_tdBallFaster;
	FLOAT m_ballSpeed;

	TickDelta m_tdShooterMode;
	MovingRectangle* m_countdownShooter;
	bool m_shooterMode;
	MovingRectangle* m_playerBullet;

	SS2DBitmap* m_bitmapMultiball;
	SS2DBitmap* m_bitmapShooter;
	SS2DBitmap* m_bitmapBatLarger;
	MovingText* m_textScoreLabel;
	MovingText* m_textScore;
public:
	AppMessage m_amQuit;
};
