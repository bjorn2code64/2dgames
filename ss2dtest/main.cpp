#include <string>

#include <d2dWindow.h>
#include <time.h>

#include <WindowSaverExt.h>
#include <Notifier.h>

#include "MenuWorld.h"
#include "InvaderWorld.h"
#include "BreakoutWorld.h"
#include "ColorsWorld.h"
#include "BaublesWorld.h"

#define APPNAME L"w32ld2d"

////////////////////////////////////////////////////////////////////////////////

class MainWindow : public D2DWindow
{
public:
	MainWindow() :
		D2DWindow(WINDOW_FLAGS_QUITONCLOSE),
		m_pBrush(NULL),
		m_windowSaver(APPNAME),
		m_worldActive(&m_worldMenu),
		m_worldMenu(m_notifier),
		m_worldInvaders(m_notifier),
		m_worldBreakout(m_notifier),
		m_worldColors(m_notifier),
		m_worldBaubles(m_notifier)
	{
		AddExt(&m_windowSaver);	// Routes Windows messages to the window position saver so it can do it's thing
	}

	~MainWindow() {
	}

	// Custom create function specifying we want our OnPaint() called automatically
	DWORD CreateAndShow(int nCmdShow) {
		RETURN_IF_ERROR(CreateOverlapped(APPNAME));

		D2DWindow::Init(m_worldActive->SS2DGetScreenSize());	// Intialise our d2d engine

		m_notifier.AddNotifyTarget(this, m_worldMenu.m_amRunInvaders);
		m_notifier.AddNotifyTarget(this, m_worldMenu.m_amRunBreakout);
		m_notifier.AddNotifyTarget(this, m_worldMenu.m_amRunColors);
		m_notifier.AddNotifyTarget(this, m_worldMenu.m_amRunTest);

		m_notifier.AddNotifyTarget(this, m_worldMenu.m_amQuit);
		m_notifier.AddNotifyTarget(this, m_worldInvaders.m_amQuit);
		m_notifier.AddNotifyTarget(this, m_worldBreakout.m_amQuit);
		m_notifier.AddNotifyTarget(this, m_worldColors.m_amQuit);
		m_notifier.AddNotifyTarget(this, m_worldBaubles.m_amQuit);

		Show(nCmdShow);
		return ERROR_SUCCESS;
	}

protected:
	// Overrides from the engine. Pass them to the active world.
	bool SS2DInit() override {
		return m_worldActive->SS2DInit();
	}

	void SS2DDeInit() override {
		m_worldActive->DeInit();
	}

	// Direct2D callbacks from the engine. Pass them to our world.
	void SS2DCreateResources(IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory) override {
		pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pBrush);
		m_worldActive->SS2DCreateResources(pDWriteFactory, pRenderTarget, pIWICFactory);
	}

	bool SS2DUpdate(ULONGLONG tick, const Point2F& ptMouse, std::queue<WindowEvent>& events) override {
		return m_worldActive->SS2DUpdate(tick, ptMouse, events);
	}

	void D2DPreRender(IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory) override {
		m_worldActive->D2DPreRender(pDWriteFactory, pRenderTarget, pIWICFactory, &m_rsFAR);
	}

	void D2DRender(ID2D1HwndRenderTarget* pRenderTarget) override {
		D2DClearScreen(m_worldActive->m_colorBackground);

		// Draw the fixed aspect rectangle
		RectF rectBounds;
		D2DGetFARRect(&rectBounds);
		pRenderTarget->DrawRectangle(rectBounds, m_pBrush);

		m_worldActive->D2DRender(pRenderTarget, m_shapeDrawFlags, &m_rsFAR);
	}

	void D2DOnResize(IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory) override {
		m_worldActive->SS2DOnResize(pDWriteFactory, pRenderTarget, pIWICFactory, &m_rsFAR);
	}

	void D2DOnDiscardResources() override {
		m_worldActive->SS2DDiscardResources();

		SafeRelease(&m_pBrush);
	}

	LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override {
		if (message == m_worldMenu.m_amRunInvaders) {
			// Disable the menu world and start Invaders world
			Stop();
			m_worldActive = &m_worldInvaders;
			D2DWindow::Init(m_worldActive->SS2DGetScreenSize());	// Intialise our d2d engine
		}
		else if (message == m_worldMenu.m_amRunBreakout) {
			// Disable the menu world and start Breakout world
			Stop();
			m_worldActive = &m_worldBreakout;
			D2DWindow::Init(m_worldActive->SS2DGetScreenSize());	// Intialise our d2d engine
		}
		else if (message == m_worldMenu.m_amRunColors) {
			// Disable the menu world and start Colors world
			Stop();
			m_worldActive = &m_worldColors;
			D2DWindow::Init(m_worldActive->SS2DGetScreenSize());	// Intialise our d2d engine
		}
		else if (message == m_worldMenu.m_amRunTest) {
			// Disable the menu world and start Test world
			Stop();
			m_worldActive = &m_worldBaubles;
			D2DWindow::Init(m_worldActive->SS2DGetScreenSize());	// Intialise our d2d engine
		}
		else if ((message == m_worldInvaders.m_amQuit) ||
			(message == m_worldBreakout.m_amQuit) ||
			(message == m_worldColors.m_amQuit) ||
			(message == m_worldBaubles.m_amQuit)
			) {
			// Disable the game world and start menu world
			Stop();
			m_worldActive = &m_worldMenu;
			D2DWindow::Init(m_worldActive->SS2DGetScreenSize());	// Intialise our d2d engine
		}
		else if (message == m_worldMenu.m_amQuit) {
			Stop();
			PostQuitMessage(0);
		}

		return __super::WndProc(hWnd, message, wParam, lParam);
	}

protected:
	MenuWorld m_worldMenu;

	InvaderWorld m_worldInvaders;
	BreakoutWorld m_worldBreakout;
	ColorsWorld m_worldColors;
	BaublesWorld m_worldBaubles;

	SS2DWorld* m_worldActive;

	ID2D1SolidColorBrush* m_pBrush;	// Fixed aspect outline brush - white

	WindowSaverExt m_windowSaver;	// Save the screen position between runs

	Notifier m_notifier;	// Notifications between the menu and the games
};

////////////////////////////////////////////////////////////////////////////////
// Main routine. Init the library, create a window and run the program.
////////////////////////////////////////////////////////////////////////////////

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	int ret = 0;
	if (SUCCEEDED(CoInitialize(NULL))) {
		Window::LibInit(hInstance);	// Initialise the library

		MainWindow w;	// Our custom Window (defined above)

		// Create and show it
		DWORD dwError = w.CreateAndShow(nCmdShow);
		if (dwError == ERROR_SUCCESS) {
			ret = w.Run();	// Run the standard Windows message loop
		}
		else {
			Window::ReportError(dwError);	// Show the error
		}

		CoUninitialize();
	}

	return ret;
}

////////////////////////////////////////////////////////////////////////////////
