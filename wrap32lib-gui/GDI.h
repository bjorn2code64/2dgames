#pragma once

#include "utils.h"

class GDI
{
public:
	GDI(HDC hDC) : m_hDC(hDC), m_bkMode(-1) {
	}

	~GDI() {
		for (auto h : m_olds) {
			::SelectObject(m_hDC, h);
		}

		if (m_bkMode != -1) {
			::SetBkMode(m_hDC, m_bkMode);
		}
	}

	void Select(HGDIOBJ h) {
		m_olds.push_back(::SelectObject(m_hDC, h));
	}

	operator HDC() { return m_hDC; }

	void SetBkMode(int mode) {
		int old = ::SetBkMode(m_hDC, mode);
		if (m_bkMode == -1) {
			m_bkMode = old;
		}
	}
protected:
	HDC m_hDC;
	std::vector<HGDIOBJ> m_olds;
	int m_bkMode;
};

class BaseBrush
{
public:
	operator HBRUSH() { return m_hBrush; }
//	operator HGDIOBJ() { return (HGDIOBJ)m_hBrush; }

	HBRUSH Select(HDC hDC) {
		return (HBRUSH)::SelectObject(hDC, m_hBrush);
	}

protected:
	HBRUSH m_hBrush;
};

class StockBrush : public BaseBrush
{
public:
	StockBrush(int id) {
		m_hBrush = (HBRUSH)::GetStockObject(id);
	}
};

class SolidBrush : public BaseBrush
{
public:
	SolidBrush(COLORREF c) {
		m_hBrush = ::CreateSolidBrush(c);
	}

	~SolidBrush() {
		::DeleteObject(m_hBrush);
	}
};

class HatchedBrush : public BaseBrush
{
public:
	HatchedBrush(int hatch, COLORREF c) {
		m_hBrush = ::CreateHatchBrush(hatch, c);
	}

	~HatchedBrush() {
		::DeleteObject(m_hBrush);
	}
};

class Pen
{
public:
	Pen(int style, int width, COLORREF c) {
		m_hPen = ::CreatePen(style, width, c);
	}

	~Pen() {
		::DeleteObject(m_hPen);
	}

	HPEN Select(HDC hDC) {
		return (HPEN)::SelectObject(hDC, m_hPen);
	}

	operator HPEN() { return m_hPen;  }
protected:
	HPEN m_hPen;
};

class Font
{
public:
	Font(LPCWSTR family, LONG height) {
		LOGFONT lf;
		::ZeroMemory(&lf, sizeof(lf));
		lf.lfWeight = FW_NORMAL;
		lf.lfHeight = height;// -12;
		wcscpy_s(lf.lfFaceName, LF_FACESIZE, family);// L"Segoe UI");
		lf.lfCharSet = DEFAULT_CHARSET;
		m_hf = ::CreateFontIndirect(&lf);
	}

	~Font() {
		::DeleteFont(m_hf);
	}

	operator HFONT() { return m_hf; }

	HFONT Select(HDC hDC) {
		return (HFONT)::SelectObject(hDC, m_hf);
	}

protected:
	HFONT m_hf;
};
