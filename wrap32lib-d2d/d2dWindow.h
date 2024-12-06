#pragma once

#include <window.h>
#include <json.h>
#include <thread.h>
#include <TimerQueue.h>

#include <dwrite.h>
#include <d2d1.h>
#include <d2d1helper.h>

#include "d2dbitmap.h"
#include "d2dwrite.h"
#include "Shape.h"

#include <string>
#include <queue>

#pragma comment(lib, "d2d1")

//using namespace D2D1;

#include <map>

class WindowEvent
{
public:
	WindowEvent(UINT msg, WPARAM wParam, LPARAM lParam) : m_msg(msg), m_wParam(wParam), m_lParam(lParam) {}

	UINT m_msg;
	WPARAM m_wParam;
	LPARAM m_lParam;
};

class D2DWindow : public Window, public EventThread
{
public:
	D2DWindow(WORD flags, Window* pParent = NULL, DWORD dwUpdateRate = 20) :
		Window(flags, pParent),
		m_pDirect2dFactory(NULL),
		m_pDWriteFactory(NULL),
		m_pIWICFactory(NULL),
		m_pRenderTarget(NULL),
		m_dwUpdateRate(dwUpdateRate),
		m_rsFAR(0, 0),
		m_updateTime(0),
		m_updateCount(0),
		m_shapeDrawFlags(0)
	{}

	~D2DWindow(void) {
		Stop();
	}

	void Init(const w32Size& size) {
		m_rsFAR.SetBaseSize(size);
		Start();
	}

	LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override {
		switch (uMsg)
		{
		case WM_DISPLAYCHANGE:
			InvalidateRect(*this, NULL, FALSE);
			break;

		case WM_DESTROY:
			Stop();
			break;

		case WM_SIZE:
			m_size = lParam;
			m_evResize.Set();
			break;

		case WM_ERASEBKGND:
			return TRUE;

		case WM_MOUSEMOVE:	// Track the mouse
		{
			w32Point mouse = lParam;
			m_rsFAR.ReverseScaleAndOffset(&mouse);
			m_ptMouse = mouse;
		}
		break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
			m_events.push(WindowEvent(uMsg, wParam, lParam));
			break;

		case WM_KEYDOWN:
			if (::GetAsyncKeyState(VK_CONTROL) & 0x8000) {
				if (wParam == 'G') {
					if (m_shapeDrawFlags & SHAPEDRAW_SHOW_GROUP_BOUNDS)
						m_shapeDrawFlags &= ~SHAPEDRAW_SHOW_GROUP_BOUNDS;
					else
						m_shapeDrawFlags |= SHAPEDRAW_SHOW_GROUP_BOUNDS;
				}
				else if (wParam == 'B') {
					if (m_shapeDrawFlags & SHAPEDRAW_SHOW_BITMAP_BOUNDS)
						m_shapeDrawFlags &= ~SHAPEDRAW_SHOW_BITMAP_BOUNDS;
					else
						m_shapeDrawFlags |= SHAPEDRAW_SHOW_BITMAP_BOUNDS;
				}
			}
		case WM_KEYUP:
			m_events.push(WindowEvent(uMsg, wParam, lParam));
			break;
		}

		return __super::WndProc(hWnd, uMsg, wParam, lParam);
	}

protected:
	HRESULT EnsureDeviceResourcesCreated() {
		HRESULT hr = S_OK;

		if (!m_pRenderTarget) {
			w32Rect rc;
			GetClientRect(*this, &rc);

			D2D1_SIZE_U size = D2D1::SizeU(
				rc.Width(),
				rc.Height()
			);

			// Create a Direct2D render target.
			hr = m_pDirect2dFactory->CreateHwndRenderTarget(
				D2D1::RenderTargetProperties(),
				D2D1::HwndRenderTargetProperties(*this, size),
				&m_pRenderTarget
			);

			if (!m_pRenderTarget) {
				return hr;
			}
			m_pRenderTarget->Resize(D2D1::SizeU(m_size.cx, m_size.cy));

			// Set rsFAR up first as it may be used in Creation of resources
			m_rsFAR.SetBounds(m_pRenderTarget->GetSize());

			SS2DCreateResources(m_pDWriteFactory, m_pRenderTarget, m_pIWICFactory);
		}

		return hr;
	}

	DWORD D2DInit() {
		// Create device independant resources here
		HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);
		if (FAILED(hr))
			return HRESULT_CODE(hr);

		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(m_pDWriteFactory),
			reinterpret_cast<IUnknown**>(&m_pDWriteFactory));

		if (FAILED(hr))
			return HRESULT_CODE(hr);

		// The factory returns the current system DPI. This is also the value it will use
		// to create its own windows.
		FLOAT dpi = (FLOAT)GetDpiForWindow(*this);
		m_rsFAR.SetDPI(dpi, dpi);

		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory,
			reinterpret_cast<void**>(&m_pIWICFactory)
		);

		if (FAILED(hr))
			return HRESULT_CODE(hr);

		return ERROR_SUCCESS;
	}

	bool OnUpdateTimer() {
		ULONGLONG updateStart = GetTickCount64();

		if (!SS2DUpdate(updateStart, m_ptMouse, m_events)) return true;
		m_events = std::queue<WindowEvent>();

		if (EnsureDeviceResourcesCreated() != S_OK)	return true;

		D2DPreRender(m_pDWriteFactory, m_pRenderTarget, m_pIWICFactory);

		m_pRenderTarget->BeginDraw();
		D2DRender(m_pRenderTarget);
		if (m_pRenderTarget->EndDraw() == D2DERR_RECREATE_TARGET) {
			D2DDiscard();	// Free these up so they're created again next time around
		}
		m_updateTime += GetTickCount64() - updateStart;
		m_updateCount++;
		return false;
	}

	bool OnResize() {
		if (m_pRenderTarget) {
			m_pRenderTarget->Resize(D2D1::SizeU(m_size.cx, m_size.cy));
			m_rsFAR.SetBounds(m_pRenderTarget->GetSize());
		}
		return false;
	}

	void D2DDiscard() {
		D2DOnDiscardResources();
		SafeRelease(&m_pRenderTarget);
	}

	virtual bool SS2DInit() { return true; }
	virtual void SS2DDeInit() {}

	virtual bool SS2DUpdate(ULONGLONG tick, const Point2F& ptMouse, std::queue<WindowEvent>& events) { return false;  }	// manipulate your data here - return true to quit

	virtual void D2DPreRender(IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory) {}	// draw the data here
	virtual void D2DRender(ID2D1HwndRenderTarget* pRenderTarget) {}	// draw the data here

	void ThreadStartup() override {
		m_updateTime = 0;
		m_updateCount = 0;

		if (CoInitialize(NULL) != S_OK) {
			return;
		}

		if (D2DInit() != ERROR_SUCCESS) {
			return;
		}

		if (!SS2DInit()) {
			return;
		}

		TimerQueue::Timer* timer = TELAddTimer(
			[this]() {	return this->OnUpdateTimer();	}
		);

		TELAddEvent(m_evResize,
			[this]() {	return this->OnResize();		}
		);

		timer->Start(0, m_dwUpdateRate);
	}

	void ThreadShutdown() override {
		SS2DDeInit();
		D2DDiscard();
		SafeRelease(&m_pIWICFactory);
		SafeRelease(&m_pDWriteFactory);
		SafeRelease(&m_pDirect2dFactory);
		CoUninitialize();
	}

	void D2DRenderPoint(D2D1_POINT_2F pt, ID2D1SolidColorBrush* pBrush, FLOAT fStrokeWidth = 1.0F) {
		m_rsFAR.Scale(&pt);

		D2D1_POINT_2F pt2 = pt;
		pt2.x += fStrokeWidth - 0.4F;

		m_pRenderTarget->DrawLine(pt, pt2, pBrush, fStrokeWidth);
	}

	void D2DClearScreen(D2D1::ColorF c)	{	m_pRenderTarget->Clear(c);	}
	void D2DGetFARRect(RectF* p)		{	m_rsFAR.GetUserRect(p);		}

	virtual void SS2DCreateResources(IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget,
		IWICImagingFactory* pIWICFactory) {}
	virtual void D2DOnDiscardResources() {}

	ULONGLONG GetAvgUpdateTime() { return (m_updateCount > 0) ? m_updateTime / m_updateCount : 0;  }

protected:
	ID2D1Factory* m_pDirect2dFactory;
	IDWriteFactory* m_pDWriteFactory;
	ID2D1HwndRenderTarget* m_pRenderTarget;
	D2DRectScaler m_rsFAR;	// Maintains Fixed Aspect Ratio rect, regardless of window size
	DirectWrite m_dw;
	IWICImagingFactory* m_pIWICFactory;
	Event m_evResize;
	w32Size m_size;
	DWORD m_dwUpdateRate;	// in ms
	Point2F m_ptMouse;				// Track the mouse position
	std::queue<WindowEvent> m_events;

	DWORD m_shapeDrawFlags;

	ULONGLONG m_updateTime;		// Time spent in update function (ms)
	ULONGLONG m_updateCount;	// Number of calls to update function
};
