#include <string>

#include <d2dWindow.h>
#include <time.h>

#include <WindowSaverExt.h>

#include "StarterWorld.h"

#define APPNAME L"ss2dstarter"

////////////////////////////////////////////////////////////////////////////////

class MainWindow : public D2DWindow
{
public:
	MainWindow() :
		D2DWindow(WINDOW_FLAGS_QUITONCLOSE),
		m_windowSaver(APPNAME)
	{
		AddExt(&m_windowSaver);	// Routes Windows messages to the window position saver so it can do it's thing
	}

	~MainWindow() {
	}

	// Custom create function specifying we want our OnPaint() called automatically
	DWORD CreateAndShow(int nCmdShow) {
		RETURN_IF_ERROR(CreateOverlapped(APPNAME));

		D2DWindow::Init(m_worldStarter.SS2DGetScreenSize());	// Intialise our d2d engine

		Show(nCmdShow);
		return ERROR_SUCCESS;
	}

protected:
	bool SS2DInit() override {
		return m_worldStarter.SS2DInit();
	}

	void SS2DDeInit() override {
		m_worldStarter.DeInit();
	}

	// Direct2D overrides from the engine Window. Pass them to our world.
	bool SS2DUpdate(ULONGLONG tick, const Point2F& ptMouse, std::queue<WindowEvent>& events) override {
		return m_worldStarter.SS2DUpdate(tick, ptMouse, events);
	}

	void D2DPreRender(const SS2DEssentials& ess) override {
		m_worldStarter.D2DPreRender(ess);
		__super::D2DPreRender(ess);
	}

	void D2DRender() override {
		D2DClearScreen(m_worldStarter.m_colorBackground);

		// Draw the fixed aspect rectangle
		RectF rectBounds;
		D2DGetFARRect(&rectBounds);
		m_ess.m_pRenderTarget->DrawRectangle(rectBounds, *m_worldStarter.GetDefaultBrush());

		m_worldStarter.D2DRender(m_ess);
		__super::D2DRender();
	}

	void D2DOnDiscardResources() override {
		m_worldStarter.SS2DDiscardResources();
	}

protected:
	StarterWorld m_worldStarter;	// The game logic
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
