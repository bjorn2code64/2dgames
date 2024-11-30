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

	const FLOAT menuPosX = 500.0f;
	const FLOAT menuPosY = 200.0f;
	const FLOAT menuWidth = m_screenWidth - (menuPosX * 2);
	const FLOAT menuHeight = m_screenHeight - (menuPosY * 2);
	const FLOAT menuTextHeight = 100;

	BouncyWorld(Notifier& notifier) :
		m_notifier(notifier),
		m_menuBackground(Point2F(menuPosX, menuPosY), menuWidth, menuHeight, 0.0f, 0, RGB(0, 0, 0)),
		m_textInvaders(L"Invaders", Point2F(menuPosX, menuPosY), menuWidth, menuTextHeight, 0.0f, 0, RGB(0, 255, 0), DWRITE_TEXT_ALIGNMENT_CENTER),
		m_textBreakout(L"Breakout", Point2F(menuPosX, menuPosY + menuTextHeight), menuWidth, menuTextHeight, 0, 0, RGB(255, 0, 0), DWRITE_TEXT_ALIGNMENT_CENTER),
		m_textColors(L"Colors", Point2F(menuPosX, menuPosY + 2 * menuTextHeight), menuWidth, menuTextHeight, 0, 0, RGB(0, 0, 255), DWRITE_TEXT_ALIGNMENT_CENTER)
	{
	}

	~BouncyWorld() {
		for (auto shape : m_movingShapes) {
			delete shape;
		}
		m_movingShapes.clear();
	}

	w32Size D2DGetScreenSize() override {
		return w32Size(m_screenWidth, m_screenHeight);
	}

	bool Init() override {
		for (int i = 0; i < m_numShapes; i++) {
			FLOAT speed = w32randf(5.0f, 15.f);
			DWORD direction = w32rand(0, 359);
			COLORREF color = RGB(w32rand(0, 256), w32rand(0, 256), w32rand(0, 256));
			if (w32rand(1)) {
				FLOAT width = w32randf(m_minRadius, m_maxRadius);
				FLOAT height = w32randf(m_minRadius, m_maxRadius);

				Point2F pos(w32randf(0, m_screenWidth - width), w32randf(0, m_screenHeight - height));

				m_movingShapes.push_back(
				new MovingRectangle(
					pos,
					width, height,
					speed,
					direction,
					color));
			}
			else {
				float radius = w32randf(m_minRadius, m_maxRadius);
				Point2F pos(w32randf(radius, m_screenWidth - radius), w32randf(radius, m_screenHeight - radius));

				m_movingShapes.push_back(new MovingCircle(
					pos,
					radius,
					speed,
					direction,
					color));
			}
		}

		for (auto shape : m_movingShapes) {
			QueueShape(shape);
		}

		QueueShape(&m_menuBackground);
		QueueShape(&m_textInvaders);
		QueueShape(&m_textBreakout);
		QueueShape(&m_textColors);

		return true;
	}

	bool D2DUpdate(ULONGLONG tick, const Point2F& ptMouse, std::queue<WindowEvent>& events) override {
		for (auto shape : m_movingShapes) {

			// Bounce the shape off the edge
			switch (shape->WillHitBounds(D2DGetScreenSize())) {
			case Position::moveResult::hitboundsleft:
				shape->BounceX();
				break;
			case Position::moveResult::hitboundsright:
				shape->BounceX();
				break;
			case Position::moveResult::hitboundstop:
				shape->BounceY();
				break;
			case Position::moveResult::hitboundsbottom:
				shape->BounceY();
				break;
			}

			// Is the mouse touching it?
			if (shape->HitTest(ptMouse)) {
				shape->SetActive(false);
			}
		}

		while (!events.empty()) {
			auto& ev = events.front();
			if (ev.m_msg == WM_LBUTTONDOWN) {
				// Left button click - where is the mouse?
				if (m_textInvaders.HitTest(ptMouse)) {
					m_notifier.Notify(m_amRunInvaders);
				}
				else if (m_textBreakout.HitTest(ptMouse)) {
					m_notifier.Notify(m_amRunBreakout);
				}
				else if (m_textColors.HitTest(ptMouse)) {
					m_notifier.Notify(m_amRunColors);
				}
			}
			else if (ev.m_msg == WM_KEYDOWN) {
				if (ev.m_wParam == VK_ESCAPE) {
					m_notifier.Notify(m_amQuit);
				}
			}
			events.pop();
		}

		return __super::D2DUpdate(tick, ptMouse, events);
	}

protected:
	Notifier& m_notifier;
	MovingRectangle m_menuBackground;
	MovingText m_textInvaders;
	MovingText m_textBreakout;
	MovingText m_textColors;
	std::vector<Shape*> m_movingShapes;

public:
	AppMessage m_amQuit;
	AppMessage m_amRunInvaders;
	AppMessage m_amRunBreakout;
	AppMessage m_amRunColors;
};