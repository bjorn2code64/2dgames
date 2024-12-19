#pragma once

#include <window.h>
#include <json.h>
#include <thread.h>
#include <TimerQueue.h>
#include <CriticalSection.h>

#include <dwrite.h>
#include <d2d1.h>
#include <d2d1helper.h>

#include "d2dwrite.h"
#include "SS2Dbitmap.h"
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
		m_dwUpdateRate(dwUpdateRate),
		m_updateTime(0),
		m_updateCount(0)
	{}

	~D2DWindow(void) {
		Stop();
	}

	void Init(const w32Size& size) {
		m_ess.m_rsFAR.SetBaseSize(size);
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
			m_ess.m_rsFAR.ReverseScaleAndOffset(&mouse);
			m_ptMouse = mouse;
		}
		break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
			m_csEvents.Enter();
			m_events.push(WindowEvent(uMsg, wParam, lParam));
			m_csEvents.Leave();
			break;

		case WM_KEYDOWN:
			if (::GetAsyncKeyState(VK_CONTROL) & 0x8000) {
				if (wParam == 'G') {
					m_ess.toggleFlag(SS2D_SHOW_GROUP_BOUNDS);
				}
				else if (wParam == 'B') {;
					m_ess.toggleFlag(SS2D_SHOW_BITMAP_BOUNDS);
				}
				else if (wParam == 'S') {
					m_ess.toggleFlag(SS2D_SHOW_STATS);
				}
			}
		case WM_KEYUP:
			m_csEvents.Enter();
			m_events.push(WindowEvent(uMsg, wParam, lParam));
			m_csEvents.Leave();
			break;
		}

		return __super::WndProc(hWnd, uMsg, wParam, lParam);
	}

protected:
	HRESULT EnsureDeviceResourcesCreated() {
		HRESULT hr = S_OK;

		if (!m_ess.m_pRenderTarget) {
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
				&m_ess.m_pRenderTarget
			);

			if (!m_ess.m_pRenderTarget) {
				return hr;
			}
			m_ess.m_pRenderTarget->Resize(D2D1::SizeU(m_size.cx, m_size.cy));

			// Set rsFAR up first as it may be used in Creation of resources
			m_ess.m_rsFAR.SetBounds(m_ess.m_pRenderTarget->GetSize());

			SS2DCreateResources(m_ess);
		}

		return hr;
	}

	DWORD D2DInit() {
		// Create device independant resources here
		HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);
		if (FAILED(hr))
			return HRESULT_CODE(hr);

		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(m_ess.m_pDWriteFactory),
			reinterpret_cast<IUnknown**>(&m_ess.m_pDWriteFactory));

		if (FAILED(hr))
			return HRESULT_CODE(hr);

		// The factory returns the current system DPI. This is also the value it will use
		// to create its own windows.
		FLOAT dpi = (FLOAT)GetDpiForWindow(*this);
		m_ess.m_rsFAR.SetDPI(dpi, dpi);

		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory,
			reinterpret_cast<void**>(&m_ess.m_pIWICFactory)
		);

		if (FAILED(hr))
			return HRESULT_CODE(hr);

		return ERROR_SUCCESS;
	}

	bool OnUpdateTimer() {
		ULONGLONG updateStart = GetTickCount64();

		m_csEvents.Enter();
		if (!SS2DUpdate(updateStart, m_ptMouse, m_events)) {
			m_csEvents.Leave();
			return true;
		}
		m_events = std::queue<WindowEvent>();
		m_csEvents.Leave();

		if (EnsureDeviceResourcesCreated() != S_OK)	return true;

		D2DPreRender(m_ess);

		m_ess.m_pRenderTarget->BeginDraw();
		D2DRender();
		if (m_ess.m_pRenderTarget->EndDraw() == D2DERR_RECREATE_TARGET) {
			D2DDiscard();	// Free these up so they're created again next time around
		}
		m_updateTime += GetTickCount64() - updateStart;
		m_updateCount++;
		return false;
	}

	bool OnResize() {
		if (m_ess.m_pRenderTarget) {
			m_ess.m_pRenderTarget->Resize(D2D1::SizeU(m_size.cx, m_size.cy));
			m_ess.m_rsFAR.SetBounds(m_ess.m_pRenderTarget->GetSize());
			D2DOnResize(m_ess);
		}
		return false;
	}

	void D2DDiscard() {
		D2DOnDiscardResources();
		SafeRelease(&m_ess.m_pRenderTarget);
	}

	virtual bool SS2DInit() { return true; }
	virtual void SS2DDeInit() {}

	virtual void D2DOnResize(const SS2DEssentials& ess) {}

	virtual bool SS2DUpdate(ULONGLONG tick, const Point2F& ptMouse, std::queue<WindowEvent>& events) { return false;  }	// manipulate your data here - return true to quit

	virtual void D2DPreRender(const SS2DEssentials& ess) {}	// draw the data here
	virtual void D2DRender() {}	// draw the data here

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
		// Clear the d2d stuff first
		D2DDiscard();
		SafeRelease(&m_ess.m_pIWICFactory);
		SafeRelease(&m_ess.m_pDWriteFactory);
		SafeRelease(&m_pDirect2dFactory);

		// Deallocate Objects
		SS2DDeInit();
		CoUninitialize();
	}

	void D2DRenderPoint(D2D1_POINT_2F pt, ID2D1SolidColorBrush* pBrush, FLOAT fStrokeWidth = 1.0F) {
		m_ess.m_rsFAR.Scale(&pt);

		D2D1_POINT_2F pt2 = pt;
		pt2.x += fStrokeWidth - 0.4F;

		m_ess.m_pRenderTarget->DrawLine(pt, pt2, pBrush, fStrokeWidth);
	}

	void D2DClearScreen(D2D1::ColorF c)	{ m_ess.m_pRenderTarget->Clear(c);	}
	void D2DGetFARRect(RectF* p)		{ m_ess.m_rsFAR.GetUserRect(p);		}

	virtual void SS2DCreateResources(const SS2DEssentials& ess) {}
	virtual void D2DOnDiscardResources() {}

	ULONGLONG GetAvgUpdateTime() { return (m_updateCount > 0) ? m_updateTime / m_updateCount : 0;  }

protected:
	// Drawing stuff
	SS2DEssentials m_ess;
	ID2D1Factory* m_pDirect2dFactory;
	DirectWrite m_dw;

	Event m_evResize;		// Notify when the window resizes
	w32Size m_size;			// Track the windo size

	DWORD m_dwUpdateRate;	// in ms
	Point2F m_ptMouse;		// Track the mouse position

	// Events stuff - keypress, etc.
	std::queue<WindowEvent> m_events;
	CriticalSection m_csEvents;		// Ensure threads don't fight over the events queue

	// Performance stats
	ULONGLONG m_updateTime;		// Time spent in update function (ms)
	ULONGLONG m_updateCount;	// Number of calls to update function
};
