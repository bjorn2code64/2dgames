#pragma once

#include "wrap32lib.h"
#include <oaidl.h>

#include <assert.h>

class variant : public VARIANT
{
public:
	variant() {
		::VariantInit(this);
	}

	variant(VARTYPE type) {
		::VariantInit(this);
		vt = type;
		if (vt == VT_BSTR)
			bstrVal = NULL;
	}

	variant(const VARIANT& v) {
		::VariantInit(this);
		::VariantCopy(this, &v);
	}

	~variant(void)
	{
		::VariantClear(this);
	}

	bool operator==(const variant& rhs)
	{
		if (vt != rhs.vt)
			return false;

		switch (vt) {
		case VT_BSTR:	return wcscmp(bstrVal, rhs.bstrVal) == 0;
		case VT_BOOL:	return boolVal == rhs.boolVal;
		case VT_UI2:	return uiVal == rhs.uiVal;
		case VT_UI4:	return uintVal == rhs.uintVal;
		case VT_R4:		return fltVal == rhs.fltVal;
		case VT_R8:		return dblVal == rhs.dblVal;
		}
		return false;
	}

	void SetType(VARTYPE type)
	{
		::VariantClear(this);
		vt = type;
		if (vt == VT_BSTR)
			bstrVal = NULL;
	}

	const variant& operator=(bool b)
	{
		assert(vt == VT_BOOL);
		boolVal = b;
		return *this;
	}

	const variant& operator=(USHORT us)
	{
		assert(vt == VT_UI2);
		uiVal = us;
		return *this;
	}

	const variant& operator=(UINT ui)
	{
		assert(vt == VT_UI4);
		uintVal = ui;
		return *this;
	}

	const variant& operator=(int i)
	{
		assert(vt == VT_I4);
		intVal = i;
		return *this;
	}

	const variant& operator=(float f)
	{
		assert(vt == VT_R4);
		fltVal = f;
		return *this;
	}

	const variant& operator=(double d)
	{
		assert(vt == VT_R8);
		dblVal = d;
		return *this;
	}

	const variant& operator=(LPCWSTR wsz)
	{
		assert(vt == VT_BSTR);
		::VariantClear(this);
		vt = VT_BSTR;

		int nLen = wcslen(wsz);
		bstrVal = SysAllocStringLen(wsz, nLen + 1);
		return *this;
	}

	const variant& operator=(const variant& rhs)
	{
		::VariantCopy(this, &rhs);
		return *this;
	}
};
