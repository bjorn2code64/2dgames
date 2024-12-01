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
		m_menuBackground(Point2F(menuPosX, menuPosY), menuWidth, menuHeight, 0.0f, 0, RGB(255, 255, 255), 0.8F)
	{
	}

	~BouncyWorld() {
		DeInit();
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

		AddMenuItem(L"Invaders", RGB(0, 128, 0), RGB(0, 255, 0), m_amRunInvaders, 95);
		AddMenuItem(L"Breakout", RGB(128, 0, 0), RGB(255, 0, 0), m_amRunBreakout, 60);
		AddMenuItem(L"Colors", RGB(0, 0, 128), RGB(0, 0, 255), m_amRunColors, 5);
		AddMenuItem(L"Exit", RGB(128, 128, 128), RGB(255, 255, 255), m_amQuit, 0);

		return true;
	}

	void DeInit() override {
		for (auto shape : m_movingShapes) {
			delete shape;
		}
		m_movingShapes.clear();

		for (auto shapepair : m_menuitems) {
			delete shapepair.first;
		}
		m_menuitems.clear();
	}

	bool D2DUpdate(ULONGLONG tick, const Point2F& ptMouse, std::queue<WindowEvent>& events) override {
		// Check all the shapes in the background
		for (auto shape : m_movingShapes) {
			// Bounce the shape off the edge
			switch (shape->WillHitBounds(D2DGetScreenSize())) {
			case Shape::moveResult::hitboundsleft:
				shape->BounceX();
				break;
			case Shape::moveResult::hitboundsright:
				shape->BounceX();
				break;
			case Shape::moveResult::hitboundstop:
				shape->BounceY();
				break;
			case Shape::moveResult::hitboundsbottom:
				shape->BounceY();
				break;
			}

			// Is the mouse touching it?
			if (shape->HitTest(ptMouse)) {
				shape->SetActive(false);
			}
		}

		// Highlight the menu item that the mouse is over 
		for (int i = 0; i < m_menuitems.size(); i += 2) {
			auto menuitem = m_menuitems[i].first;
			auto menuitemhighlighted = m_menuitems[i + 1].first;

			if (menuitem->HitTest(ptMouse)) {
				menuitem->SetActive(false);
				menuitemhighlighted->SetActive(true);
			}
			else {
				menuitem->SetActive(true);
				menuitemhighlighted->SetActive(false);
			}
		}

		// Keyboard and mouse functions
		while (!events.empty()) {
			auto& ev = events.front();
			if (ev.m_msg == WM_LBUTTONDOWN) {	// left mouse button pressed?
				for (int i = 0; i < m_menuitems.size(); i += 2) {	// which menu did we click on?
					auto menuitem = m_menuitems[i].first;
					if (menuitem->HitTest(ptMouse)) {
						m_notifier.Notify(m_menuitems[i].second);
					}
				}
			}
			else if (ev.m_msg == WM_KEYDOWN) {	// key pressed?
				if (ev.m_wParam == VK_ESCAPE) {
					m_notifier.Notify(m_amQuit);
				}
			}
			events.pop();
		}

		return __super::D2DUpdate(tick, ptMouse, events);
	}

protected:
	void AddMenuItem(LPCWSTR text, COLORREF color, COLORREF highlight, AppMessage am, int percentDone) {
		size_t menuitems = m_menuitems.size() / 2;
//		auto p = new MovingRectangle(
//			Point2F(menuPosX, menuPosY + (FLOAT)menuitems * menuTextHeight),
//			menuWidth * percentDone / 100, menuTextHeight, 0, 0,
//			color,
//			0.6f);
//		QueueShape(p);

		// Create two texts, a normal one and a highlight one. Show the normal one and hide the highlight one.
		auto t = new MovingText(
			text,
			Point2F(menuPosX, menuPosY + (FLOAT)menuitems * menuTextHeight),
			menuWidth, menuTextHeight, 0, 0, DWRITE_TEXT_ALIGNMENT_CENTER,
			color);
		m_menuitems.push_back(std::make_pair(t, am));
		QueueShape(t);

		t = new MovingText(
			text,
			Point2F(menuPosX, menuPosY + (FLOAT)menuitems * menuTextHeight),
			menuWidth, menuTextHeight + 5.0F, 0, 0, DWRITE_TEXT_ALIGNMENT_CENTER,
			highlight);
		m_menuitems.push_back(std::make_pair(t, am));
		QueueShape(t, false);
	}

protected:
	Notifier& m_notifier;				// For sending updates to other parts of the program 
	MovingRectangle m_menuBackground;	// transparent box around menu items.
	std::vector<std::pair<MovingText*, AppMessage>> m_menuitems;	// the menu texts
	std::vector<Shape*> m_movingShapes;	// moving shapes in the background

public:
	AppMessage m_amRunInvaders;
	AppMessage m_amRunBreakout;
	AppMessage m_amRunColors;
	AppMessage m_amQuit;
};