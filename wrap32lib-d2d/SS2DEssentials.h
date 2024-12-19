#pragma once

#include <dwrite.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <wincodec.h>

#include "d2dtypes.h"

#define SS2D_SHOW_GROUP_BOUNDS		0x0001
#define SS2D_SHOW_BITMAP_BOUNDS		0x0002
#define SS2D_SHOW_STATS				0x0004

class SS2DEssentials {
public:
	SS2DEssentials() :
		m_pDWriteFactory(NULL),
		m_pRenderTarget(NULL),
		m_pIWICFactory(NULL),
		m_rsFAR(0, 0),
		m_ss2dFlags(0)
	{
	}

	void toggleFlag(DWORD flag) {
		if (m_ss2dFlags & flag)
			m_ss2dFlags &= ~flag;
		else
			m_ss2dFlags |= flag;
	}

	IDWriteFactory* m_pDWriteFactory;
	IWICImagingFactory* m_pIWICFactory;
	ID2D1HwndRenderTarget* m_pRenderTarget;
	SS2DRectScaler m_rsFAR;
	DWORD m_ss2dFlags;
};
