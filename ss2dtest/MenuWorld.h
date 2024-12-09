#pragma once

#include <SS2DWorld.h>

class MenuWorld : public SS2DWorld
{
public:
	const int m_numShapes = 5000;
	const float m_maxRadius = 60.0f;
	const float m_minRadius = 10.0f;

	const FLOAT menuPosX = 500.0f;
	const FLOAT menuPosY = 200.0f;
	const FLOAT menuWidth = SS2DGetScreenSize().cx - (menuPosX * 2);
	const FLOAT menuHeight = SS2DGetScreenSize().cy - (menuPosY * 2);
	const FLOAT menuTextHeight = 100;

	MenuWorld(Notifier& notifier) :
		m_notifier(notifier)
	{
	}

	~MenuWorld() {
	}

	bool SS2DInit() override {
		for (int i = 0; i < m_numShapes; i++) {
			FLOAT speed = w32randf(5.0f, 15.f);
			DWORD direction = w32rand(0, 359);
			COLORREF color = RGB(w32rand(0, 256), w32rand(0, 256), w32rand(0, 256));
			auto br = NewResourceBrush(color);
			if (w32rand(1)) {
				FLOAT width = w32randf(m_minRadius, m_maxRadius);
				FLOAT height = w32randf(m_minRadius, m_maxRadius);

				m_movingShapes.push_back(
				NewMovingRectangle(
					w32randf(0, SS2DGetScreenSize().cx - width),
					w32randf(0, SS2DGetScreenSize().cy - height),
					width, height,
					speed,
					direction,
					br));
			}
			else {
				float radius = w32randf(m_minRadius, m_maxRadius);

				m_movingShapes.push_back(NewMovingCircle(
					w32randf(radius, SS2DGetScreenSize().cx - radius),
					w32randf(radius, SS2DGetScreenSize().cy - radius),
					radius,
					speed,
					direction,
					br));
			}
		}

		m_brushMenu = NewResourceBrush(RGB(255, 255, 255), 0.8f);

		m_menuBackground = NewMovingRectangle(menuPosX, menuPosY, menuWidth, menuHeight, 0.0f, 0, m_brushMenu, 0.8f);

		AddMenuItem(L"Invaders", RGB(0, 128, 0), RGB(0, 255, 0), m_amRunInvaders, 95);
		AddMenuItem(L"Breakout", RGB(128, 0, 0), RGB(255, 0, 0), m_amRunBreakout, 60);
		AddMenuItem(L"Colors", RGB(0, 0, 128), RGB(0, 0, 255), m_amRunColors, 5);
		AddMenuItem(L"Test", RGB(128, 0, 128), RGB(255, 0, 255), m_amRunTest, 5);
		AddMenuItem(L"Exit", RGB(128, 128, 128), RGB(255, 255, 255), m_amQuit, 0);

		return true;
	}

	void SS2DDeInit() override {
		m_menuitems.clear();
	}

	bool SS2DUpdate(ULONGLONG tick, const Point2F& ptMouse, std::queue<WindowEvent>& events) override {
		// Check all the shapes in the background
		for (auto shape : m_movingShapes) {
			// Bounce the shape off the edge
			switch (shape->WillHitBounds(SS2DGetScreenSize())) {
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

		return __super::SS2DUpdate(tick, ptMouse, events);
	}

protected:
	void AddMenuItem(LPCWSTR text, COLORREF color, COLORREF highlight, AppMessage am, int percentDone) {
		size_t menuitems = m_menuitems.size() / 2;

		// Create two texts, a normal one and a highlight one. Show the normal one and hide the highlight one.
		auto t = NewMovingText(
			text,
			menuPosX, menuPosY + (FLOAT)menuitems * menuTextHeight,
			menuWidth, menuTextHeight, 0, 0, DWRITE_TEXT_ALIGNMENT_CENTER,
			NewResourceBrush(color));
		m_menuitems.push_back(std::make_pair(t, am));

		t = NewMovingText(
			text,
			menuPosX, menuPosY + (FLOAT)menuitems * menuTextHeight,
			menuWidth, menuTextHeight + 5.0F, 0, 0, DWRITE_TEXT_ALIGNMENT_CENTER,
			NewResourceBrush(highlight));
		m_menuitems.push_back(std::make_pair(t, am));
	}

protected:
	Notifier& m_notifier;				// For sending updates to other parts of the program 

	SS2DBrush* m_brushMenu;
	MovingRectangle* m_menuBackground;	// transparent box around menu items.
	std::vector<std::pair<MovingText*, AppMessage>> m_menuitems;	// the menu texts
	std::vector<Shape*> m_movingShapes;	// moving shapes in the background

public:
	AppMessage m_amRunInvaders;
	AppMessage m_amRunBreakout;
	AppMessage m_amRunColors;
	AppMessage m_amRunTest;
	AppMessage m_amQuit;
};