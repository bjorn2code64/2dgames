#pragma once

#include <D2DWorld.h>

class ColorsWorld : public D2DWorld
{
public:
	static const int m_screenWidth = 800;		// 16 columns
	static const int m_screenHeight = 1000;		// 20 rows

	static const int playerWidth = 50;

	static const int boardWidth = m_screenWidth / playerWidth;
	static const int boardHeight = m_screenHeight / playerWidth;

	class Board {
	public:
		~Board() {
			for (auto& m : m_board) {
				delete m.second;
			}
		}

		void Set(MovingGroup* p) {
			for (auto& m : p->GetMembers()) {
				Point2F pos = m->GetPos();
				pos += p->GetPos();
				m->SetPos(pos);
				Set((int)m->GetPos().y / playerWidth, (int)m->GetPos().x / playerWidth, (MovingRectangle*)m);
			}
		}

		void Set(int row, int col, MovingRectangle* p) {
			m_board[getIndex(row, col)] = p;
		}

		MovingRectangle* Get(int row, int col) {
			auto it = m_board.find(getIndex(row, col));
			return (it != m_board.end()) ? it->second : NULL;
		}

		bool WillHit(MovingGroup* pGroup) {
			Point2F pos = pGroup->GetPos();
			pGroup->MovePos(pos);	// next frame position

			for (auto m : pGroup->GetMembers()) {
				RectF r;
				m->GetBoundingBox(&r, pos);
				if (HitTest(r))
					return true;
			}
			return false;
		}

		bool HitTest(RectF& rect) {
			for (auto piece : m_board) {
				if (piece.second->HitTest(rect)) {
					return true;
				}
			}
			return false;
		}

	protected:
		int getIndex(int row, int col) {
			return col + row * boardWidth;
		}
	protected:
		std::map<DWORD, MovingRectangle*> m_board;
	};

	ColorsWorld(Notifier& notifier) : m_notifier(notifier), m_player(NULL) {
	}

	bool Init() override {
		Drop();
		return true;
	}

	w32Size D2DGetScreenSize() override {
		return w32Size(m_screenWidth, m_screenHeight);
	}

	bool D2DUpdate(ULONGLONG tick, const Point2F& ptMouse, std::queue<WindowEvent>& events) override {
		// Check player controls
		UpdateCheckPlayerKeys();

		switch (m_player->WillHitBounds(D2DGetScreenSize()))
		{
		case Position::moveResult::hitboundsbottom:
			m_board.Set(m_player);
			m_player->SetSpeed(0);
			RemoveShape(m_player);
			Drop();
			break;

		case Position::moveResult::hitboundsleft:
		case Position::moveResult::hitboundsright:
			m_player->SetDirectionInDeg(180);
			break;

		default:
			break;
		}

		// Check if we've hit anything else
		if (m_board.WillHit(m_player)) {
			m_board.Set(m_player);
			m_player->SetSpeed(0);
			RemoveShape(m_player);
			Drop();
		}

		// Check for quit (Escape)
		while (!events.empty()) {
			auto& ev = events.front();
			if (ev.m_msg == WM_KEYDOWN) {
				if (ev.m_wParam == VK_ESCAPE) {
					m_notifier.Notify(m_amQuit);
				}
			}
			events.pop();
		}

		return __super::D2DUpdate(tick, ptMouse, events);
	}

	void Drop() {
		m_player = new MovingGroup(Point2F(400.0f, 0.0f), 4.0f, 180);

		m_player->Add(new MovingRectangle(Point2F(0, 0), playerWidth, playerWidth, 0, 0, RGB(255, 0, 0)));
		m_player->Add(new MovingRectangle(Point2F(0, 50), playerWidth, playerWidth, 0, 0, RGB(0, 255, 0)));
		m_player->Add(new MovingRectangle(Point2F(0, 100), playerWidth, playerWidth, 0, 0, RGB(0, 0, 255)));

		QueueShape(m_player);
	}

	bool UpdateCheckPlayerKeys() {
		// Left/right?
		if (KeyDown(VK_LEFT)) {
			m_player->SetDirectionInDeg(225);
		}
		else if (KeyDown(VK_RIGHT)) {
			m_player->SetDirectionInDeg(135);
		}
		else {	// no direction key
			if (m_player->GetDirectionInDeg() != 180) {				// Is the player still moving sideways?
				Point2F pos = m_player->GetPos();
				int x = (int)(pos.x / playerWidth);	// column we're on now

				m_player->MovePos(pos);
				int newx = (int)(pos.x / playerWidth);	// column we'll be on next frame

				if (x != newx) {
					if (m_player->GetDirectionInDeg() == 135) {
						m_player->SetPos(Point2F((FLOAT)(newx * playerWidth), pos.y));			// snap to column
					}
					else {
						m_player->SetPos(Point2F((FLOAT)(x * playerWidth), pos.y));			// snap to column
					}
					m_player->SetDirectionInDeg(180);				// Stop moving sideways
				}
			}
		}

		return true;
	}

protected:
	MovingGroup* m_player;

	Board m_board;

	Notifier& m_notifier;
public:
	AppMessage m_amQuit;
};


//		char buff[256];
//		_snprintf_s(buff, 256, "%d,%d\n", x, newx);
//		OutputDebugStringA(buff);

