
#include "Window.h"

class Resizer
{
public:
	Resizer() : m_pParent(nullptr) {}

	void Init(Window* pParent) {
		m_pParent = pParent;
		w32Rect r;
		::GetClientRect(*pParent, r);
		m_size.cx = r.right;
		m_size.cy = r.bottom;
	}

	class Item {
	public:
		static const WORD scaleX = 0x0001;
		static const WORD scaleY = 0x0002;
		static const WORD moveX = 0x0004;
		static const WORD moveY = 0x0008;

		Item(Window* p, w32Rect r, WORD wOpts) : m_p(p), m_rect(r), m_wOpts(wOpts) {}

		void Resize(SIZE sizeDiff) {
			int left = m_rect.left;
			if (m_wOpts & moveX) {
				left += sizeDiff.cx;
			}

			int top = m_rect.top;
			if (m_wOpts & moveY) {
				top += sizeDiff.cy;
			}

			int width = m_rect.right - m_rect.left + 1;
			if (m_wOpts & scaleX) {
				width += sizeDiff.cx;
			}

			int height = m_rect.bottom - m_rect.top + 1;
			if (m_wOpts & scaleY) {
				height += sizeDiff.cy;
			}

			::SetWindowPos(
				*m_p,
				NULL,
				left, top,
				width, height,
				SWP_NOZORDER);
		}
	protected:
		Window* m_p;
		w32Rect m_rect;
		WORD m_wOpts;
	};

	void Add(Window* pClient, WORD wOpts) {
		w32Rect r;
		::GetClientRect(*pClient, &r);
		pClient->ClientToScreen(&r);
		m_pParent->ScreenToClient(&r);
		m_items.push_back(Item(pClient, r, wOpts));
	}

	void OnSize(int cx, int cy) {
		// diff between first x and new x
		SIZE sizeDiff;
		sizeDiff.cx = cx - m_size.cx;
		sizeDiff.cy = cy - m_size.cy;
		for (Item& i : m_items) {
			i.Resize(sizeDiff);
		}
	}

	void OnSize(RECT* r) {
		OnSize(r->right, r->bottom);
	}

protected:
	Window* m_pParent;
	w32Size m_size;
	std::vector<Item> m_items;
};