#pragma once

#include <wrap32lib.h>

#include <d2d1.h>
#include <wincodec.h>

#include "d2dtypes.h"

#pragma comment(lib, "windowscodecs")

class d2dBitmap {
public:
	d2dBitmap() : m_pBitmap(NULL) {
	}

	~d2dBitmap() {
		Clear();
	}

	void Clear() {
		SafeRelease(&m_pBitmap);
	}

	bool IsValid() const { return m_pBitmap != NULL;  }

	w32Size GetSize() const {
		if (m_pBitmap) {
			D2D1_SIZE_F sf = m_pBitmap->GetSize();
			return w32Size((long)sf.width, (long)sf.height);
		}
		return w32Size(0, 0);
	}

	void Render(ID2D1RenderTarget* pRenderTarget, const D2D1_RECT_F& rectBounds) const {
		if (m_pBitmap) {
			pRenderTarget->DrawBitmap(
				m_pBitmap,
				rectBounds
			);
		}
	}

	HRESULT LoadFromFile (
		ID2D1RenderTarget* pRenderTarget,
		IWICImagingFactory* pIWICFactory,
		PCWSTR uri,
		UINT destinationWidth,
		UINT destinationHeight
	)
	{
		HRESULT hr = S_OK;

		IWICBitmapDecoder* pDecoder = NULL;
		IWICBitmapFrameDecode* pSource = NULL;
		IWICStream* pStream = NULL;
		IWICFormatConverter* pConverter = NULL;
		IWICBitmapScaler* pScaler = NULL;

		hr = pIWICFactory->CreateDecoderFromFilename(
			uri,
			NULL,
			GENERIC_READ,
			WICDecodeMetadataCacheOnLoad,
			&pDecoder
		);

		if (SUCCEEDED(hr)) {			// Create the initial frame.
			hr = pDecoder->GetFrame(0, &pSource);
		}

		if (SUCCEEDED(hr)) { 	// Convert the image format to 32bppPBGRA (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
			hr = pIWICFactory->CreateFormatConverter(&pConverter);
		}

		if (SUCCEEDED(hr)) {	// If a new width or height was specified, create an IWICBitmapScaler and use it to resize the image.
			if (destinationWidth != 0 || destinationHeight != 0) {
				UINT originalWidth, originalHeight;
				hr = pSource->GetSize(&originalWidth, &originalHeight);
				if (SUCCEEDED(hr)) {
					if (destinationWidth == 0) {
						FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
						destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
					}
					else if (destinationHeight == 0) {
						FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
						destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
					}

					hr = pIWICFactory->CreateBitmapScaler(&pScaler);
					if (SUCCEEDED(hr)) {
						hr = pScaler->Initialize(
							pSource,
							destinationWidth,
							destinationHeight,
							WICBitmapInterpolationModeCubic
						);
					}
					if (SUCCEEDED(hr)) {
						hr = pConverter->Initialize(
							pScaler,
							GUID_WICPixelFormat32bppPBGRA,
							WICBitmapDitherTypeNone,
							NULL,
							0.f,
							WICBitmapPaletteTypeMedianCut
						);
					}
				}
			}
			else { // Don't scale the image.
				hr = pConverter->Initialize(
					pSource,
					GUID_WICPixelFormat32bppPBGRA,
					WICBitmapDitherTypeNone,
					NULL,
					0.f,
					WICBitmapPaletteTypeMedianCut
				);
			}
		}
		if (SUCCEEDED(hr)) {	// Create a Direct2D bitmap from the WIC bitmap.
			hr = pRenderTarget->CreateBitmapFromWicBitmap(
				pConverter,
				NULL,
				&m_pBitmap
			);
		}

		SafeRelease(&pDecoder);
		SafeRelease(&pSource);
		SafeRelease(&pStream);
		SafeRelease(&pConverter);
		SafeRelease(&pScaler);

		return hr;
	}

protected:
	ID2D1Bitmap* m_pBitmap;
};
