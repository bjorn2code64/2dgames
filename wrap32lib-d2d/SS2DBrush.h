#pragma once

#include <wrap32lib.h>

#include <d2d1.h>

class SS2DBrush {
public:
	SS2DBrush(COLORREF cr, FLOAT alpha = 1.0f) : m_pBrush(NULL), m_cr(cr), m_alpha(alpha)
	{}

	~SS2DBrush() {
		Clear();
	}

	COLORREF GetColor() { return m_cr;  }

	void Clear() {
		SafeRelease(&m_pBrush);
	}

	operator ID2D1SolidColorBrush* () { return m_pBrush;  }

	void Create(ID2D1HwndRenderTarget* pRenderTarget) {
		UINT32 rgb =
			(m_cr & 0x000000ff) << 16 |
			(m_cr & 0x0000ff00) |
			(m_cr & 0x00ff0000) >> 16;	// RGB => BGR

		pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(rgb, m_alpha), &m_pBrush);
	}

protected:
	ID2D1SolidColorBrush* m_pBrush;
	COLORREF m_cr;
	FLOAT m_alpha;
};
