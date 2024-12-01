#pragma once

#include <D2DWorld.h>

#include <array>

class ColorsWorld : public D2DWorld
{
public:
	const int playerWidth = 50;

	const int m_screenWidth = 800;		// 16 columns
	const int m_screenHeight = 1000;		// 20 rows

	const int boardWidth = m_screenWidth / playerWidth;
	const int boardHeight = m_screenHeight / playerWidth;

	class Board {
	public:
		static const int boardWidth = 16;
		static const int boardHeight = 20;

		~Board() {
			for (auto& c : m_board) {
				for (auto& r : c.second) {
					if (r.second) {
						delete r.second;
					}
				}
			}
		}

		void Set(int row, int col, Shape* p) {
			m_board[row][col] = p;
		}

		Shape* Get(int row, int col) {
			auto it = m_board.find(row);
			if (it == m_board.end())
				return NULL;
			auto itC = it->second.find(col);
			return (itC == it->second.end()) ? NULL : itC->second;
		}

		bool WillHit(MovingGroup* pGroup) {
			Point2F pos = pGroup->GetPos();
			pGroup->MovePos(pos);	// next frame position

			for (auto m : pGroup->GetChildren()) {
				RectF r;
				m->GetBoundingBox(&r, pos);
				if (HitTest(r))
					return true;
			}
			return false;
		}

		bool HitTest(RectF& rect) {
			for (auto& c : m_board) {
				for (auto& r : c.second) {
					if (r.second->HitTest(rect)) {
						return true;
					}
				}
			}
			return false;
		}

	protected:
		std::map<int, std::map<int, Shape*>> m_board;
	};

	ColorsWorld(Notifier& notifier) : 
		m_notifier(notifier), 
		m_fallingGroup(Point2F(400.0f, 0.0f), 4.0f, 180) {
	}

	bool Init() override {
		Drop();
		return true;
	}

	w32Size D2DGetScreenSize() override {
		return w32Size(m_screenWidth, m_screenHeight);
	}

	bool D2DUpdate(ULONGLONG tick, const Point2F& ptMouse, std::queue<WindowEvent>& events) override {

		switch (m_fallingGroup.WillHitBounds(D2DGetScreenSize()))
		{
		case Shape::moveResult::hitboundsbottom:
			for (auto mr : m_fallingGroup.GetChildren()) {
				m_board.Set((int)(mr->GetPos().y / playerWidth), (int)(mr->GetPos().x / playerWidth), (MovingRectangle*)mr);
			}
			Drop();
			break;

		case Shape::moveResult::hitboundsleft:
		case Shape::moveResult::hitboundsright:
			m_fallingGroup.SetDirectionInDeg(180);
			break;

		default:
			break;
		}

		// Check if we've hit anything else
		if (m_board.WillHit(&m_fallingGroup)) {
			for (auto mr : m_fallingGroup.GetChildren()) {
				m_board.Set((int)(mr->GetPos().y / playerWidth), (int)(mr->GetPos().x / playerWidth), (MovingRectangle*)mr);
			}
			Drop();
		}

		// Check for quit (Escape)
		while (!events.empty()) {
			auto& ev = events.front();
			if (ev.m_msg == WM_KEYDOWN) {
				if (ev.m_wParam == VK_ESCAPE) {
					m_notifier.Notify(m_amQuit);	// QUIT
				}
				else if (ev.m_wParam == VK_RIGHT) {	// Move right
					if (m_fallingGroup.GetPos().x < m_screenWidth - playerWidth) {
						m_fallingGroup.OffsetPos(Point2F((FLOAT)playerWidth, 0));
					}
				}
				else if (ev.m_wParam == VK_LEFT) {	// Move left
					if (m_fallingGroup.GetPos().x >= playerWidth) {
						m_fallingGroup.OffsetPos(Point2F((FLOAT)-playerWidth, 0));
					}
				}
			}
			events.pop();
		}

		return __super::D2DUpdate(tick, ptMouse, events);
	}

	void Drop() {
		// Add the blocks to the engine as they won't be in the group anymore.
		for (auto m : m_fallingGroup.GetChildren()) {
			QueueShape(m);
		}
		// Remove them from the group
		m_fallingGroup.RemoveAllChildren();
		// Remove the group from the engine
		RemoveShape(&m_fallingGroup, false);

		// Reset the group
		m_fallingGroup.SetPos(Point2F(400, 0));
		m_fallingGroup.AddChild(new MovingRectangle(Point2F(0, 0), playerWidth, playerWidth, 0, 0, RGB(255, 0, 0)));
		m_fallingGroup.AddChild(new MovingRectangle(Point2F(0, 50), playerWidth, playerWidth, 0, 0, RGB(0, 255, 0)));
		m_fallingGroup.AddChild(new MovingRectangle(Point2F(0, 100), playerWidth, playerWidth, 0, 0, RGB(0, 0, 255)));

		// Add it back to the engine
		QueueShape(&m_fallingGroup);
	}

protected:
	MovingGroup m_fallingGroup;
	Board m_board;
	Notifier& m_notifier;

public:
	AppMessage m_amQuit;
};
