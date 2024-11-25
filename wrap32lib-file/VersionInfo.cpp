#include "VersionInfo.h"
#include "FileHandler.h"

#pragma comment(lib, "version")

VersionInfo::VersionInfo(LPCWSTR szPath)
{
	m_pVersion = NULL;

	FileHandler fh;
	if (!szPath) {
		fh.SetToRunningModule();
		szPath = fh;
	}

	DWORD dummyZero;
	DWORD versionSize = GetFileVersionInfoSize(szPath, &dummyZero);
	if (versionSize == 0)
		return;

	m_pVersion = malloc(versionSize);
	if (!m_pVersion)
		return;

	if (!GetFileVersionInfo(szPath, 0, versionSize, m_pVersion))
	{
		free(m_pVersion);
		m_pVersion = NULL;
	}
}

VersionInfo::~VersionInfo(void)
{
	if (m_pVersion) {
		free(m_pVersion);
	}
}

BOOL VersionInfo::GetVersionNumbers(std::vector<int>& v)
{
	if (!m_pVersion)
		return FALSE;

	UINT length;
	VS_FIXEDFILEINFO* pFixInfo;
	if (!VerQueryValue(m_pVersion, L"\\", (LPVOID*)&pFixInfo, &length))
		return FALSE;

	v.push_back(pFixInfo->dwProductVersionMS >> 16);
	v.push_back(pFixInfo->dwProductVersionMS & 0x0000ffff);
	v.push_back(pFixInfo->dwProductVersionLS >> 16);
	v.push_back(pFixInfo->dwProductVersionLS & 0x0000ffff);

	return TRUE;
}

BOOL VersionInfo::GetVersionNumbers(int* pVersions, int nLen)
{
	if (!m_pVersion)
		return FALSE;

	UINT length;
	VS_FIXEDFILEINFO* pFixInfo;
	if (!VerQueryValue(m_pVersion, L"\\", (LPVOID*)&pFixInfo, &length))
		return FALSE;

	if (nLen >= 1) pVersions[0] = pFixInfo->dwProductVersionMS >> 16;
	if (nLen >= 2) pVersions[1] = pFixInfo->dwProductVersionMS & 0x0000ffff;
	if (nLen >= 3) pVersions[2] = pFixInfo->dwProductVersionLS >> 16;
	if (nLen >= 4) pVersions[3] = pFixInfo->dwProductVersionLS & 0x0000ffff;

	return TRUE;
}
