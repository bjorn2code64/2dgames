#pragma once

#include <SS2DWorld.h>

#define c_screenWidth	600
#define c_screenHeight	900
#define c_blockSize		60
#define c_fallingSpeed	2
#define c_boardWidth	(c_screenWidth / c_blockSize)
#define c_boardHeight	(c_screenHeight / c_blockSize)
#define c_colorsInUse	6

class ColorsWorld : public SS2DWorld
{
public:
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

		bool MarkMatches() {
			std::vector<Shape*> deletedShapes;
			bool ret = ResolveHoriz(deletedShapes);
			ret |= ResolveVert(deletedShapes);
			return ret;
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
			bool found = true;
			while (found) {
				found = ResolveHoriz(deleteShapes);
				found |= ResolveVert(deleteShapes);
			}
		}

		bool MarkedForDeletion() {
			for (auto x : m_board) {
				for (auto y : x.second) {
					if (y.second->GetUserData() == 1) {
						return true;
					}
				}
			}
			return false;
		}

		void FadeDeleteds(std::vector<Shape*>& deleteShapes) {
			for (auto x : m_board) {
				for (auto y : x.second) {
					if (y.second->GetUserData() == 1) {
						MovingRectangle* p = (MovingRectangle *)y.second;
						p->OffsetPos(Point2F(0, 0.5f));
						p->SetHeight(p->GetHeight() - 1.0f);
						if (p->GetHeight() < 2.0f) {
							deleteShapes.push_back(RemoveSquareAndDrop((int)(p->GetPos().y / c_blockSize), (int)(p->GetPos().x / c_blockSize)));
						}
					}
				}
			}

		}

		bool CheckMatched(std::vector<Shape*>& matched, std::vector<Shape*>& deleteShapes) {
			if (matched.size() >= 3) {
				for (auto b : matched) {
					b->SetUserData(1);	// mark for deletion
				}
				return true;
			}
			return false;
		}

		bool ResolveHoriz(std::vector<Shape*>& deleteShapes) {
			bool found = false;
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
								found |= CheckMatched(matched, deleteShapes);
								matched.clear();
								matched.push_back(sq);
								break;
							}
						}
					}
					else {
						found |= CheckMatched(matched, deleteShapes);
						matched.clear();
					}
				}
				found |= CheckMatched(matched, deleteShapes);
			}
			return found;
		}

		bool ResolveVert(std::vector<Shape*>& deleteShapes) {
			bool found = false;
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
								found |= CheckMatched(matched, deleteShapes);
								matched.clear();
								matched.push_back(sq);
								break;
							}
						}
					}
					else {
						found |= CheckMatched(matched, deleteShapes);
						matched.clear();
					}
				}
				found |= CheckMatched(matched, deleteShapes);
			}

			return found;
		}

	protected:
		std::map<int, std::map<int, Shape*>> m_board;
	};

	ColorsWorld(Notifier& notifier) : 
		m_notifier(notifier), 
		m_fallingGroup(400.0f, 0.0f, c_fallingSpeed, 180) {
		SS2DSetScreenSize(w32Size(c_screenWidth, c_screenHeight));
	}

	bool SS2DInit() override {
		Drop();
		return true;
	}

	void SS2DDeInit() {
		RemoveShape(&m_fallingGroup);
		m_fallingGroup.RemoveAllChildren(true);
		m_board.Clear();
	}

	bool SS2DUpdate(ULONGLONG tick, const Point2F& ptMouse, std::queue<WindowEvent>& events) override {
		if (m_board.MarkedForDeletion()) {
			// Do something deletey animationy
			std::vector<Shape*> deleteShapes;
			m_board.FadeDeleteds(deleteShapes);
			for (auto s : deleteShapes) {
				RemoveShape(s, true);
			}

			if (!m_board.MarkedForDeletion()) {
				// Check for more matches first
				if (!m_board.MarkMatches()) {
					Drop();
				}
			}
		}
		else {

			// Check for bottom of screen
			if (m_fallingGroup.WillHitBounds(SS2DGetScreenSize()) == Shape::moveResult::hitboundsbottom) {
				m_fallingGroup.SetPos(Point2F(m_fallingGroup.GetPos().x, c_screenHeight - 3 * c_blockSize));
				if (!AddToBoard()) {
					Drop();	// only drop if no matches were made
				}
			}

			// Check if we've hit anything else
			if (m_board.WillHit(&m_fallingGroup)) {
				FLOAT y = m_fallingGroup.GetPos().y;	// align the falling group with the grid
				int yInt = (int)(((y - 1) / c_blockSize) + 1) * c_blockSize;
				m_fallingGroup.Offset(Point2F(0, yInt - y));

				if (!AddToBoard()) {
					Drop();	// only drop if no matches were made
				}
			}
		}

		// Check for quit (Escape)
		while (!events.empty()) {
			auto& ev = events.front();
			if (ev.m_msg == WM_KEYDOWN) {
				if (ev.m_wParam == VK_ESCAPE) {
					m_notifier.Notify(m_amQuit);	// QUIT
				}
				else if (ev.m_wParam == VK_RIGHT) {	// Move right
					if ((m_fallingGroup.GetPos().x < SS2DGetScreenSize().cx - c_blockSize) &&
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
				else if (ev.m_wParam == VK_DOWN) {
					m_fallingGroup.SetSpeed(c_fallingSpeed * 4);
				}
			}
			else if (ev.m_msg == WM_KEYUP) {
				if (ev.m_wParam == VK_DOWN) {
					m_fallingGroup.SetSpeed(c_fallingSpeed);
				}
			}
			events.pop();
		}

		return __super::SS2DUpdate(tick, ptMouse, events);
	}

	bool AddToBoard() {
		// Add the blocks to the board
		for (auto mr : m_fallingGroup.GetChildren()) {
			m_board.Set((int)(mr->GetPos().y / c_blockSize), (int)(mr->GetPos().x / c_blockSize), (MovingRectangle*)mr);
			QueueShape(mr);
		}

		// Remove them from the group
		m_fallingGroup.RemoveAllChildren();
		// Remove the group from the engine
		RemoveShape(&m_fallingGroup);

		if (m_board.MarkMatches()) {
			m_fallingGroup.SetActive(false);
			return true;
		}

		return false;
	}

	void Drop() {
		m_fallingGroup.SetActive(true);

		// Reset the group
		m_fallingGroup.SetPos(Point2F((c_screenWidth / 2 ) - c_blockSize, 0));
		m_fallingGroup.AddChild(
			new MovingRectangle(0, 0, c_blockSize, c_blockSize, 0, 0, c_colorsAvailable[w32rand((DWORD)c_colorsInUse - 1)])
		);
		m_fallingGroup.AddChild(
			new MovingRectangle(0, c_blockSize, c_blockSize, c_blockSize, 0, 0, c_colorsAvailable[w32rand((DWORD)c_colorsInUse - 1)])
		);
		m_fallingGroup.AddChild(
			new MovingRectangle(0, c_blockSize * 2, c_blockSize, c_blockSize, 0, 0, c_colorsAvailable[w32rand((DWORD)c_colorsInUse - 1)])
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
