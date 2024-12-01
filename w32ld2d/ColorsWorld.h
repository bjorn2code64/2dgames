#pragma once

#include <D2DWorld.h>

#define c_blockSize		50.0f
#define c_fallingSpeed	5.0f

class ColorsWorld : public D2DWorld
{
public:
	const w32Size c_screenSize = w32Size(800, 1000);	// 16 x 20 blocks

	const std::vector<COLORREF> c_colorsAvailable = {
		RGB(255, 0, 0),
		RGB(0, 255, 0),
		RGB(0, 0, 255),
		RGB(255, 255, 0),
		RGB(255, 0, 255),
		RGB(0, 255, 255),
		RGB(255, 255, 255)
	};

	class Board {
	public:
		static const int boardWidth = 16;
		static const int boardHeight = 20;

		~Board() {
			Clear();
		}

		void Clear() {
			m_board.clear();
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

		bool IsClear(int row, int col, int depth) {
			for (int y = 0; y < depth; y++) {
				if (Get(row + y, col) != NULL)
					return false;
			}
			return true;
		}

		bool WillHit(MovingGroup* pGroup) {
			for (auto m : pGroup->GetChildren()) {
				Point2F pos = m->GetPos();
				m->MovePos(pos);
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

		Shape* RemoveSquare(int row, int col) {
			auto it = m_board.find(row);
			if (it == m_board.end())	return NULL;
			Shape* ret = it->second[col];
			it->second.erase(col);
			return ret;
		}

		Shape* RemoveSquareAndDrop(int row, int col) {
			Shape* ret = RemoveSquare(row, col);

			// drop all the squares above this one until a gap
			while (--row >= 0) {
				auto sq = Get(row, col);
				if (!sq) {
					break;
				}

				RemoveSquare(row, col);
				Set(row + 1, col, sq);
				sq->OffsetPos(Point2F(0.0f, c_blockSize));
			}

			return ret;
		}

		void Resolve(std::vector<Shape*>& deleteShapes) {
			ResolveHoriz(deleteShapes);
			ResolveVert(deleteShapes);
		}

		void CheckMatched(std::vector<Shape*>& matched, std::vector<Shape*>& deleteShapes) {
			if (matched.size() >= 3) {
				for (auto it = matched.rbegin(); it != matched.rend(); ++it) {
					deleteShapes.push_back(RemoveSquareAndDrop((int)((*it)->GetPos().y / c_blockSize), (int)((*it)->GetPos().x / c_blockSize)));
				}
			}
		}

		void ResolveHoriz(std::vector<Shape*>& deleteShapes) {
			for (int row = 0; row < 20; row++) {
				std::vector<Shape*> matched;
				for (int col = 0; col < 16; col++) {
					auto sq = Get(row, col);
					if (sq) {
						COLORREF crThis = sq->GetColor();
						matched.push_back(sq);
						for (auto m : matched) {
							if (m->GetColor() != crThis) {
								matched.pop_back();	// remove the square that broke the chain
								CheckMatched(matched, deleteShapes);
								matched.clear();
								matched.push_back(sq);
								break;
							}
						}
					}
					else {
						CheckMatched(matched, deleteShapes);
						matched.clear();
					}
				}
				CheckMatched(matched, deleteShapes);
			}
		}

		void ResolveVert(std::vector<Shape*>& deleteShapes) {
			for (int col = 0; col < 16; col++) {
				std::vector<Shape*> matched;
				for (int row = 0; row < 20; row++) {
					auto sq = Get(row, col);
					if (sq) {
						COLORREF crThis = sq->GetColor();
						matched.push_back(sq);
						for (auto m : matched) {
							if (m->GetColor() != crThis) {
								matched.pop_back();	// remove the square that broke the chain
								CheckMatched(matched, deleteShapes);
								matched.clear();
								matched.push_back(sq);
								break;
							}
						}
					}
					else {
						CheckMatched(matched, deleteShapes);
						matched.clear();
					}
				}
				CheckMatched(matched, deleteShapes);
			}
		}

	protected:
		std::map<int, std::map<int, Shape*>> m_board;
	};

	ColorsWorld(Notifier& notifier) : 
		m_notifier(notifier), 
		m_fallingGroup(Point2F(400.0f, 0.0f), c_fallingSpeed, 180) {
	}

	bool Init() override {
		Drop();
		return true;
	}

	void DeInit() {
		RemoveShape(&m_fallingGroup, false);
		m_fallingGroup.RemoveAllChildren(true);
		m_board.Clear();
	}

	w32Size D2DGetScreenSize() override {
		return c_screenSize;
	}

	bool D2DUpdate(ULONGLONG tick, const Point2F& ptMouse, std::queue<WindowEvent>& events) override {
		// Check for bottom of screen
		if (m_fallingGroup.WillHitBounds(D2DGetScreenSize()) == Shape::moveResult::hitboundsbottom) {
			Drop();
		}

		// Check if we've hit anything else
		if (m_board.WillHit(&m_fallingGroup)) {
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
					if ((m_fallingGroup.GetPos().x < c_screenSize.cx - c_blockSize) &&
						m_board.IsClear((int)(m_fallingGroup.GetPos().y / c_blockSize), (int)(m_fallingGroup.GetPos().x / c_blockSize + 1), 4))
					{
						m_fallingGroup.OffsetPos(Point2F((FLOAT)c_blockSize, 0));
					}
				}
				else if (ev.m_wParam == VK_LEFT) {	// Move left
					if ((m_fallingGroup.GetPos().x >= c_blockSize) &&
						m_board.IsClear((int)(m_fallingGroup.GetPos().y / c_blockSize), (int)(m_fallingGroup.GetPos().x / c_blockSize - 1), 4))
					{
						m_fallingGroup.OffsetPos(Point2F((FLOAT)-c_blockSize, 0));
					}
				}
				else if (ev.m_wParam == VK_CONTROL) {	// Rotate colors
					Shape* p = m_fallingGroup.GetChildren()[2];
					m_fallingGroup.RemoveChild(p);
					m_fallingGroup.InsertChild(p);

					FLOAT y = 0.0f;
					for (auto c : m_fallingGroup.GetChildren()) {
						c->SetPos(Point2F(0.0f, y));
						y += c_blockSize;
					}
				}
			}
			events.pop();
		}

		return __super::D2DUpdate(tick, ptMouse, events);
	}

	void Drop() {
		// Add the blocks to the board
		for (auto mr : m_fallingGroup.GetChildren()) {
			m_board.Set((int)(mr->GetPos().y / c_blockSize), (int)(mr->GetPos().x / c_blockSize), (MovingRectangle*)mr);
		}

		std::vector<Shape*> deletedShapes;
		m_board.Resolve(deletedShapes);
		for (auto s : deletedShapes) {
			RemoveShape(s);
		}

		// Add the blocks to the engine as they won't be in the group anymore.
		for (auto m : m_fallingGroup.GetChildren()) {
			// Only queue to the engine the shapes that weren't removed during Resolve()
			if (std::find(deletedShapes.begin(), deletedShapes.end(), m) == deletedShapes.end()) {
				QueueShape(m);
			}
		}
		// Remove them from the group
		m_fallingGroup.RemoveAllChildren();
		// Remove the group from the engine
		RemoveShape(&m_fallingGroup, false);

		// Reset the group
		m_fallingGroup.SetPos(Point2F(400, 0));
		m_fallingGroup.AddChild(
			new MovingRectangle(Point2F(0, 0), c_blockSize, c_blockSize, 0, 0, c_colorsAvailable[w32rand((DWORD)c_colorsAvailable.size() - 1)])
		);
		m_fallingGroup.AddChild(
			new MovingRectangle(Point2F(0, 50), c_blockSize, c_blockSize, 0, 0, c_colorsAvailable[w32rand((DWORD)c_colorsAvailable.size() - 1)])
		);
		m_fallingGroup.AddChild(
			new MovingRectangle(Point2F(0, 100), c_blockSize, c_blockSize, 0, 0, c_colorsAvailable[w32rand((DWORD)c_colorsAvailable.size() - 1)])
		);

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
