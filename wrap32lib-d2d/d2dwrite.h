#pragma once

#include <wrap32lib.h>
#include <json.h>

#include <d2d1.h>
#include <dwrite.h>

#include "d2dtypes.h"

#pragma comment(lib, "Dwrite")

class DWTextFormat
{
public:
	DWTextFormat() : m_pTextFormat(NULL) {}
	~DWTextFormat() {
		SafeRelease(&m_pTextFormat);
	}

	HRESULT Init(IDWriteFactory* pDirectWriteFactory, LPCWSTR wszFontFamily, FLOAT fontSize, DWRITE_TEXT_ALIGNMENT align, bool bold, bool italic) {
		SafeRelease(&m_pTextFormat);

		HRESULT hr = pDirectWriteFactory->CreateTextFormat(
			wszFontFamily,             // Font family name.
			NULL,                       // Font collection (NULL sets it to use the system font collection).
			bold? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_REGULAR,
			italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			fontSize,
			L"en-us",
			&m_pTextFormat
		);

		// Center align (horizontally) the text.
		if (SUCCEEDED(hr)) {
			hr = m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		}

		if (SUCCEEDED(hr)) {
			hr = m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		}

		if (SUCCEEDED(hr)) {
			hr = m_pTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
		}

		return hr;
	}

	operator IDWriteTextFormat* () const { return m_pTextFormat; }

protected:
	IDWriteTextFormat* m_pTextFormat;

public:
	static std::vector<std::wstring> m_fontNames;
};

class DirectWrite
{
public:
	DirectWrite() : m_pDirectWriteFactory(NULL) {
		DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&m_pDirectWriteFactory));
	}

	~DirectWrite() {
		SafeRelease(&m_pDirectWriteFactory);
	}

	operator IDWriteFactory* const () { return m_pDirectWriteFactory; }

	HRESULT InitFontNames() {
		IDWriteFontCollection* pFontCollection;
		HRESULT hr = m_pDirectWriteFactory->GetSystemFontCollection(&pFontCollection);
		if (!SUCCEEDED(hr))
			return hr;

		UINT32 familyCount = pFontCollection->GetFontFamilyCount();
		for (UINT32 i = 0; i < familyCount; i++) {
			IDWriteFontFamily* pFontFamily = NULL;
			if (pFontCollection->GetFontFamily(i, &pFontFamily) != S_OK)
				continue;

			IDWriteLocalizedStrings* pFamilyNames = NULL;
			if (pFontFamily->GetFamilyNames(&pFamilyNames) != S_OK)
				continue;

			UINT32 index = 0;
			BOOL exists = false;
			wchar_t localeName[LOCALE_NAME_MAX_LENGTH];
			int defaultLocaleSuccess = GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH);
			if (defaultLocaleSuccess && (pFamilyNames->FindLocaleName(localeName, &index, &exists) != S_OK))
				continue;

			if (!exists) {
				if (pFamilyNames->FindLocaleName(L"en-us", &index, &exists) != S_OK)
					continue;
			}

			if (!exists)
				index = 0;

			UINT32 length = 0;
			if (pFamilyNames->GetStringLength(index, &length) != S_OK)
				continue;

			wchar_t* name = new wchar_t[(size_t)length + 1];
			if (pFamilyNames->GetString(index, name, length + 1) != S_OK) {
				delete[] name;
				continue;
			}

			DWTextFormat::m_fontNames.push_back(name);
			delete[] name;
		}

		SafeRelease(&pFontCollection);
		return S_OK;
	}

protected:
	IDWriteFactory* m_pDirectWriteFactory;
};

class DWStyle {
public:
	enum class alignment {
		left,
		centre,
		right
	};

	DWStyle() : m_alignment(alignment::left), m_fontSize(18.0f), m_bold(false), m_italic(false), m_hFont(NULL) {
	}

	DWStyle(const DWStyle& rhs) {
		*this = rhs;
	}

	DWStyle(LPCWSTR fontName, FLOAT fontSize) :
		m_fontName(fontName), m_fontSize(fontSize),
		m_alignment(alignment::left), m_hFont(NULL) {
	}

	~DWStyle() {
		if (m_hFont)
			::DeleteObject(m_hFont);
	}

	const DWStyle& operator=(const DWStyle& rhs) {
		m_fontName = rhs.m_fontName;
		m_fontSize = rhs.m_fontSize;
		m_alignment = rhs.m_alignment;
		m_bold = rhs.m_bold;
		m_italic = rhs.m_italic;
		m_hFont = NULL;
		return *this;
	}

	bool LoadJSON(json::jsonObject* joStyle) {
		json::jsonString* jsFontName = joStyle->GetStringProperty(L"font_name");
		json::jsonInt* jiFontSize = joStyle->GetIntProperty(L"font_size");
		json::jsonInt* jiAlignment = joStyle->GetIntProperty(L"alignment");
		if (!jsFontName || !jiFontSize)
			return false;

		m_fontName = jsFontName->GetValue();
		m_fontSize = (FLOAT)jiFontSize->GetValue();
		m_alignment = jiAlignment ? (alignment)jiAlignment->GetValue() : alignment::left;

		json::jsonBool* jbBold = joStyle->GetBoolProperty(L"bold");
		m_bold = jbBold ? jbBold->GetValue() : false;
		json::jsonBool* jbItalic = joStyle->GetBoolProperty(L"italic");
		m_italic = jbItalic ? jbItalic->GetValue() : false;

		return true;
	}

	void SaveJSON(json::jsonBuilder& jbs) {
		jbs.Add(L"font_name", m_fontName);
		jbs.Add(L"font_size", (DWORD)m_fontSize);
		if (m_alignment != alignment::left)
			jbs.Add(L"alignment", (DWORD)m_alignment);

		if (m_bold)			jbs.Add(L"bold", m_bold);
		if (m_italic)		jbs.Add(L"italic", m_italic);
	}

	void D2DCreateResources(ID2D1HwndRenderTarget* pRenderTarget, DirectWrite& dw, SS2DRectScaler& rsFar) {
		FLOAT fFontSize = m_fontSize;
		rsFar.ScaleNoOffset(&fFontSize);
		m_dwtf.Init(dw, m_fontName.c_str(), fFontSize, (DWRITE_TEXT_ALIGNMENT)m_alignment, m_bold, m_italic);
	}

	void D2DOnDiscardResources() {
	}

	// GDI Support
	HFONT GDISelectFont(HDC hDC) {
		if (!m_hFont) {
			LOGFONT LogFont;

			LogFont.lfHeight = -MulDiv((int)m_fontSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
			LogFont.lfWidth = 0;
			LogFont.lfEscapement = 0;
			LogFont.lfOrientation = 0;
			LogFont.lfWeight = m_bold ? FW_BOLD : FW_NORMAL;
			LogFont.lfItalic = m_italic;
			LogFont.lfUnderline = 0;
			LogFont.lfStrikeOut = 0;
			LogFont.lfCharSet = ANSI_CHARSET;
			LogFont.lfOutPrecision = OUT_TT_PRECIS;
			LogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
			LogFont.lfQuality = DEFAULT_QUALITY;
			LogFont.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
			wcscpy_s(LogFont.lfFaceName, 32, m_fontName.c_str());
			::SetBkMode(hDC, OPAQUE);

			m_hFont = ::CreateFontIndirect(&LogFont);
		}
		return SelectFont(hDC, m_hFont);
	}

	// Styles
	std::wstring m_fontName;
	FLOAT m_fontSize;
	alignment m_alignment;
	bool m_bold;
	bool m_italic;

	// Resources
	DWTextFormat m_dwtf;

	// GDI Resources
	HFONT m_hFont;
};

