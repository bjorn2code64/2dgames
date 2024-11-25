#pragma once

#include <wrap32lib.h>
#include <commdlg.h>
#include <utils.h>

#include <deque>
#include <vector>

#define LINES_PER_PAGE			60
#define PORT_DEFAULT_PRINTER	-1

class GDIPrinter
{
public:
	GDIPrinter() {
	}

	void PrintLine(LPCTSTR sz)
	{
		m_lineCache.push_back(sz);

		if (m_lineCache.size() >= LINES_PER_PAGE) {
			std::deque<std::wstring> page;
			for (int i = 0; i < LINES_PER_PAGE; i++) {
				page.push_back(m_lineCache.at(i));
			}

			if (PrintPage(page)) {
				m_lineCache.erase(m_lineCache.begin(), m_lineCache.begin() + LINES_PER_PAGE);
			}
		}
	}

/*	bool PrintFile(LPCTSTR sz) {
		FILE* fp = _wfopen(sz, L"r");
		std::vector<std::wstring> page;
		if (!fp)
			return false;

		char buff[1024];
		while (fgets(buff, 1024, fp)) {
			int nLen = strlen(buff) - 1;
			if ((nLen >= 0) && (buff[nLen] == '\n'))
				buff[nLen] = '\0';
			page.AddTail(buff);
		}
		fclose(fp);
		return PrintPage(page);
	}*/

	bool PrintPage(std::deque<std::wstring>& pageLines) {
		PRINTDLG pd = { 0 };
		pd.lStructSize = sizeof(pd);
		pd.hwndOwner = NULL; // ??? hWnd;
		pd.Flags = PD_RETURNDC;

		if (!PrintDlg(&pd))
			return false;

		HDC hDC = pd.hDC;
		if (!hDC)
			return false;

		// setup font specifics
		LOGFONT LogFont;

		LogFont.lfHeight = -MulDiv(10, GetDeviceCaps(hDC, LOGPIXELSY), 72);
		LogFont.lfWidth = 0;
		LogFont.lfEscapement = 0;
		LogFont.lfOrientation = 0;
		LogFont.lfWeight = 0;
		LogFont.lfItalic = false;
		LogFont.lfUnderline = 0;
		LogFont.lfStrikeOut = 0;
		LogFont.lfCharSet = ANSI_CHARSET;
		LogFont.lfOutPrecision = OUT_TT_PRECIS;
		LogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		LogFont.lfQuality = DEFAULT_QUALITY;
		LogFont.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
		wcscpy(LogFont.lfFaceName, L"Courier New");
		::SetBkMode(hDC, OPAQUE);

		HFONT hFont = ::CreateFontIndirect(&LogFont);
		// ok, we've build the font, now use it
		HFONT hFontOld = (HFONT)::SelectObject(hDC, hFont);

		// Initialise print document details  
		DOCINFO di;
		::ZeroMemory(&di, sizeof(DOCINFO));
		di.cbSize = sizeof(DOCINFO);

		// application title appears in the spooler view
		di.lpszDocName = L"document";	//??

//		PrintInfo Info;
		// Begin a new print job
		int status = ::StartDoc(hDC, &di);

		// Get the printing extents and store in the m_rectDraw field of a CPrintInfo object
		w32Rect rectDraw(0, 0, ::GetDeviceCaps(hDC, HORZRES), ::GetDeviceCaps(hDC, VERTRES));
		int curPage;

		//		FILE* inputFile = fopen(szFile, "r");
		//		if (!inputFile)
		//			return FALSE;

		std::wstring buf;
		std::wstring lastLine;

		// Loop until all pages are printed.
		// NB the test (lastLine.length() != 0) is required as the a line could technically
		// span several pages. If a file with 0 '\n' characters is read in one go, then the
		// end of the input file will have been reached but the variable lastLine could
		// theoretically contain several pages of text that needs to be printed.
		for (UINT page = 0; ((status >= 0) && (!pageLines.empty() || (lastLine.size() != 0))); page++)
		{
			// begin new page
			status = ::StartPage(hDC);

			if (status >= 0)
			{
				SelectObject(hDC, hFont);
				curPage = page;

				// calc how much text fits on one page
				std::wstring pageOfText(lastLine);
				int nHeight;

				w32Rect r(rectDraw);

				// Initialise the page with the remains of the text from the last line of the last page.
				if (pageOfText.size() == 0)
				{
					r.bottom = r.top;
				}
				else
				{
					nHeight = ::DrawText(hDC, pageOfText.c_str(),
						pageOfText.size(),
						r,
						DT_CALCRECT | DT_WORDBREAK | DT_NOCLIP | DT_EXPANDTABS);

					r.right = rectDraw.right;
					pageOfText += L"\r\n";
				}

				// While there is more input keeping adding text to the printer page a line at a time.
				while ((r.bottom < rectDraw.bottom) && !pageLines.empty())
				{
//					strcpy(buf, LPCTSTR(pageLines.RemoveHead()));
					buf = pageLines.front();
					while ((buf.size() > 0) && (buf.at(buf.size() - 1) == L'\n'))
						buf.pop_back();

					lastLine = buf;
					pageOfText += lastLine;

					nHeight = ::DrawText(hDC, pageOfText.c_str(),
						pageOfText.size(),
						r,
						DT_CALCRECT | DT_WORDBREAK | DT_NOCLIP | DT_EXPANDTABS);

					r.right = rectDraw.right;

					// Add (carriage return | new line) before getting the next line of text.
					pageOfText += L"\r\n";
				}

				// Don't need the (carriage return | new line) on the last line.
				pageOfText.pop_back();
				pageOfText.pop_back();

				int lastLineDrop = 0;   // Number of characters to drop from last line.
				int lastLineAdd = 0;   // Number of characters we are thinking about changing drop by.

				if (r.bottom >= rectDraw.bottom)
				{
					// Repeatedly do a binary chop on the last line until we find out
					// how much of this line we can include on this page.

					// NB This is necessary because we may have a very long input line (ie no '\n' character),
					// that is mapped on to several lines on a printer page. NB printer page lines are word wrapped.
					lastLineDrop = lastLine.size();
					lastLineAdd = lastLineDrop;

					do
					{
						while ((r.bottom >= rectDraw.bottom) && (lastLineAdd > 0))
						{
							lastLineAdd = lastLineAdd / 2;
							nHeight = ::DrawText(hDC, pageOfText.c_str(), pageOfText.size() - lastLineDrop + lastLineAdd, r, DT_CALCRECT | DT_WORDBREAK | DT_NOCLIP | DT_EXPANDTABS);
							r.right = rectDraw.right;
						}
						lastLineDrop -= lastLineAdd;
						while ((r.bottom < rectDraw.bottom) && (lastLineAdd > 0))
						{
							lastLineAdd = lastLineAdd / 2;
							nHeight = ::DrawText(hDC, pageOfText.c_str(), pageOfText.size() - lastLineDrop + lastLineAdd, r, DT_CALCRECT | DT_WORDBREAK | DT_NOCLIP | DT_EXPANDTABS);
							r.right = rectDraw.right;
							lastLineDrop -= lastLineAdd;
						}
						lastLineDrop += lastLineAdd;
					} while (lastLineAdd > 0);
				}

				// Draw the line of text and update the last line string
				// so that it does not include text that has been printed.
				::DrawText(hDC, pageOfText.c_str(), pageOfText.size() - lastLineDrop, r, DT_WORDBREAK | DT_NOCLIP | DT_EXPANDTABS);
				lastLine.erase(0, lastLine.size() - lastLineDrop);

				// print that text

				// end page
				status = ::EndPage(hDC);
			}
		}

		//		fclose(inputFile);

				// end a print job
		if (status >= 0)
			::EndDoc(hDC);
		else			// abort job.
			::AbortDoc(hDC);

		::SelectObject(hDC, hFontOld);

		return true;
	}

protected:
	std::vector<std::wstring> m_lineCache;
};
