
#include "ResourceStrings.h"
#include "Window.h"

BOOL ResourceString::Load() {
	if (m_sz)
		return TRUE;

	LPWSTR sz;
	int nLen = LoadString(Window::GetInstance(), m_nID, (LPWSTR)&sz, 0);
	if (!nLen)
		return FALSE;

	m_sz = new wchar_t[(size_t)nLen + 1];
	LoadString(Window::GetInstance(), m_nID, m_sz, nLen + 1);
	return TRUE;
}

ResourceString::operator LPCWSTR () const
{
	return m_sz ? m_sz : L"";
}

BOOL ResourceString::Get(LPCWSTR& sz) {
	sz = m_sz;
	return m_sz != NULL;
}
