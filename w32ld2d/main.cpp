#include <string>

#include <d2dWindow.h>
#include <time.h>

#include <WindowSaverExt.h>

#include "BouncyWorld.h"
#include "InvaderWorld.h"
#include "BreakoutWorld.h"

#define APPNAME L"w32ld2d"

////////////////////////////////////////////////////////////////////////////////

class MainWindow : public D2DWindow
{
public:
	MainWindow() :
		D2DWindow(WINDOW_FLAGS_QUITONCLOSE),
		m_pBrush(NULL),
		m_windowSaver(APPNAME)
	{
		srand((unsigned int)time(NULL));	// Stop random numbers being the same every time
		w32seed();

		AddExt(&m_windowSaver);	// Routes Windows messages to the window position saver so it can do it's thing
	}

	~MainWindow() {
	}

	// Custom create function specifying we want our OnPaint() called automatically
	DWORD CreateAndShow(int nCmdShow) {
		RETURN_IF_ERROR(CreateOverlapped(APPNAME));

		m_world.Init();	// Intialise the world
		D2DWindow::Init(m_world.D2DGetScreenSize());	// Intialise our d2d engine

		Show(nCmdShow);
		return ERROR_SUCCESS;
	}

protected:
	// Direct2D callbacks from the engine. Pass them to our world.
	void D2DOnCreateResources(IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory) override  {
		pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pBrush);
		m_world.D2DCreateResources(pDWriteFactory, pRenderTarget, pIWICFactory, &m_rsFAR);
	}

	bool D2DUpdate(ULONGLONG tick, const Point2F& ptMouse, std::queue<WindowEvent>& events) override {
		return m_world.D2DUpdate(tick, ptMouse, events);
	}

	void D2DPreRender(IDWriteFactory* pDWriteFactory, ID2D1HwndRenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory) override {
		m_world.D2DPreRender(pDWriteFactory, pRenderTarget, pIWICFactory, &m_rsFAR);
	}

	void D2DRender(ID2D1HwndRenderTarget* pRenderTarget) override {
		D2DClearScreen(m_world.m_colorBackground);

		// Draw the fixed aspect rectangle
		RectF rectBounds;
		D2DGetFARRect(&rectBounds);
		pRenderTarget->DrawRectangle(rectBounds, m_pBrush);

		m_world.D2DRender(pRenderTarget, &m_rsFAR);
	}

	void D2DOnDiscardResources() override {
		m_world.D2DDiscardResources();

		SafeRelease(&m_pBrush);
	}


protected:
// Instance just one of the following worlds

	// BouncyWorld m_world;
	// InvaderWorld m_world;
	BreakoutWorld m_world;

	ID2D1SolidColorBrush* m_pBrush;	// Fixed aspect outline brush - white

	WindowSaverExt m_windowSaver;	// Save the screen position between runs
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
