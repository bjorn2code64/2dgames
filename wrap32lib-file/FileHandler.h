#pragma once

#include <Shlobj.h>
#include <Winver.h>
#include <string>
#include <unicodemultibyte.h>
#include <Shlwapi.h>
#include <json.h>

#include "File.h"

class FileHandler
{
public:
	FileHandler(LPCWSTR szFile, LPCWSTR szDefaultExt = NULL) : m_strFile(szFile)		{
		// If this is a valid filename (it's got something in it)
		if (szFile && szFile[0] && szDefaultExt) {
			// If no ext, assign the passed one
			wchar_t ext[256];
			_wsplitpath_s(szFile, NULL, 0, NULL, 0, NULL, 0, ext, 256);
			if (!ext[0]) {
				m_strFile += szDefaultExt;
			}
		}
	}
	FileHandler(int nFolder, LPCWSTR szFileName, LPCWSTR szExt, LPCWSTR szFolderName = NULL) {
		SetSystemPath(nFolder, szFileName, szExt, szFolderName);
	}

	FileHandler() {}

	unsigned char* AllocContents(unsigned long* pLong = NULL, bool textMode = false, bool nullterminate = false) {
		FILE* fp = NULL;
		if (_wfopen_s(&fp, m_strFile.c_str(), textMode ? L"r" : L"rb") || !fp) {
			return NULL;
		}

		fseek(fp, 0, SEEK_END);
		unsigned long fsize = ftell(fp) + (nullterminate ? 1 : 0);
		rewind(fp);
		unsigned char* buf = new unsigned char[fsize];
		size_t got = fread(buf, 1, fsize, fp);
		if (!got) {
			delete[] buf;
			return NULL;
		}

		if (nullterminate)
			buf[got++] = '\0';

		if (pLong) {
			*pLong = (unsigned long)got;
		}

		fclose(fp);
		return buf;
	}

	wchar_t* AllocContentsW(unsigned long* pLong = NULL) const {
		FILE* fp = NULL;
		if (_wfopen_s(&fp, m_strFile.c_str(), L"rb") || !fp) {
			return NULL;
		}

		fseek(fp, 0, SEEK_END);
		unsigned long fsize = ftell(fp);
		rewind(fp);
		fsize /= sizeof(wchar_t);
		wchar_t* buf = new wchar_t[fsize];
		if (fread(buf, sizeof(wchar_t), fsize, fp) != fsize) {
			delete[] buf;
			return NULL;
		}
		if (pLong) {
			*pLong = fsize;
		}

		fclose(fp);
		return buf;
	}

	// Filename
	const FileHandler& operator=(LPCWSTR sz)	{	m_strFile = sz;	return *this;	}
	void Clear()								{	m_strFile.clear();				}
	bool IsEmpty() const						{	return m_strFile.empty();		}
	operator LPCWSTR() const					{	return m_strFile.c_str();		}
	void SetToRunningModule() {
		wchar_t buff[MAX_PATH + 1];
		::GetModuleFileName(NULL, buff, MAX_PATH + 1);
		m_strFile = buff;
	}

	void SetToWorkingFolder() {
		wchar_t buff[MAX_PATH + 1];
		::GetCurrentDirectory(NULL, buff);
		m_strFile = buff;

	}

	void SetToTempFile(LPCWSTR fileid) {
		wchar_t buffPath[MAX_PATH];
		GetTempPath(MAX_PATH, buffPath);

		wchar_t buff[MAX_PATH];
		::GetTempFileName(buffPath, fileid, 0, buff);

		m_strFile = buff;
	}

	void SetSystemPath(int nFolder, LPCWSTR szFileName, LPCWSTR szExt, LPCWSTR szFolderName = NULL) {
		wchar_t buff[MAX_PATH];
		::SHGetSpecialFolderPath(NULL, buff, nFolder, FALSE);
		if (szFolderName) {
			wcscat_s(buff, MAX_PATH, L"\\");
			wcscat_s(buff, MAX_PATH, szFolderName);
			CreateDirectory(buff, NULL);
		}

		if (szFileName) {
			wcscat_s(buff, MAX_PATH, L"\\");
			wcscat_s(buff, MAX_PATH, szFileName);
		}

		if (szExt) {
			wcscat_s(buff, MAX_PATH, L".");
			wcscat_s(buff, MAX_PATH, szExt);
		}
		m_strFile = buff;
	}

	void Append(LPCWSTR sz) {
		std::wstring gs = L"\\";

		wchar_t ch = (m_strFile.length() > 0) ? m_strFile.back() : 0;
		if ((ch != '\\') && (ch != '/'))
			m_strFile += gs;

		m_strFile += sz;
	}

	FILE* wfopen(LPCWSTR mode) const {
		FILE* fp;
		return _wfopen_s(&fp, m_strFile.c_str(), mode) ? NULL : fp;
	}

	bool Load(File& f) {
		return f.ReadString(m_strFile);
	}

	bool Save(File& f) const {
		return f.WriteString(m_strFile.c_str());
	}

	void SetFilenameAndExt(LPCWSTR szFile, LPCWSTR szExt) {
		GetPath(m_strFile);
		m_strFile += szFile;
		m_strFile += L".";
		m_strFile += szExt;
	}

	bool GetPath(std::wstring& str) const {
		wchar_t drive[256];
		wchar_t dir[MAX_PATH];
		_wsplitpath_s(m_strFile.c_str(), drive, 256, dir, MAX_PATH, NULL, 0, NULL, 0);
		str = drive;
		str += dir;
		return true;
	}

	bool GetFilename(std::wstring& str) const {
		wchar_t name[MAX_PATH];
		wchar_t ext[16];
		_wsplitpath_s(m_strFile.c_str(), NULL, 0, NULL, 0, name, MAX_PATH, ext, 16);
		str = name;
		str += ext;
		return true;
	}

	bool DeleteFile() const {
		return ::DeleteFile(m_strFile.c_str()) != 0;
	}

	bool CopyFile(LPCWSTR szDestPath, bool bFailIfExists) const {
		return ::CopyFile(m_strFile.c_str(), szDestPath, bFailIfExists ? TRUE : FALSE) ? true : false;
	}

	bool Exists() const {
		DWORD dwAttrib = GetFileAttributes(m_strFile.c_str());
		return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
	}

	ULONGLONG GetSize() const {
		WIN32_FILE_ATTRIBUTE_DATA fileInfo;
		if (!GetFileAttributesEx(m_strFile.c_str(), GetFileExInfoStandard, (void*)&fileInfo))
			return 0;
		return ((ULONGLONG)fileInfo.nFileSizeHigh << 32) | (ULONGLONG)fileInfo.nFileSizeLow;
	}
	
protected:
	std::wstring m_strFile;
};

class IniFileHandler : public FileHandler
{
public:
	IniFileHandler(LPCWSTR szAppName, LPCWSTR szFileName = NULL, int nFolder = CSIDL_COMMON_APPDATA) {
		SetSystemPath(CSIDL_COMMON_APPDATA, szFileName ? szFileName : szAppName, L"ini", szAppName);
	}

	std::wstring GetString(LPCWSTR szSection, LPCWSTR szName, LPCWSTR szDefault = L"") const {
		wchar_t buff[1024];
		GetPrivateProfileString(szSection, szName, szDefault, buff, 1024, m_strFile.c_str());
		return buff;
	}

	std::string GetString(LPCWSTR szSection, LPCWSTR szName, LPCSTR szDefault = "") const {
		wchar_t buff[1024];
		GetPrivateProfileString(szSection, szName, UnicodeMultibyte(szDefault), buff, 1024, m_strFile.c_str());
		return LPCSTR(UnicodeMultibyte(buff));
	}

	void GetString(wchar_t* pRet, int nLen, LPCWSTR szSection, LPCWSTR szName, LPCWSTR szDefault = L"") const {
		GetPrivateProfileString(szSection, szName, szDefault, pRet, nLen, m_strFile.c_str());
	}

	int GetInt(LPCWSTR szSection, LPCWSTR szName, int nDefault = 0) const {
		wchar_t buff[64];
		_snwprintf_s(buff, 64, _TRUNCATE, L"%d", nDefault);
		buff[63] = '\0';
		GetPrivateProfileString(szSection, szName, buff, buff, 64, m_strFile.c_str());
		return _wtoi(buff);
	}

	DWORD GetDWORD(LPCWSTR szSection, LPCWSTR szName, DWORD dwDefault = 0) const {
		wchar_t buff[64];
		GetPrivateProfileString(szSection, szName, NULL, buff, 64, m_strFile.c_str());
		DWORD dw;
		return (swscanf_s(buff, L"%u", &dw) == 1) ? dw : dwDefault;
	}

	double GetDouble(LPCWSTR szSection, LPCWSTR szName, double dDefault = 0.0) const {
		wchar_t buff[64];
		_snwprintf_s(buff, 64, _TRUNCATE, L"%0.15g", dDefault);
		buff[63] = '\0';
		GetPrivateProfileString(szSection, szName, buff, buff, 64, m_strFile.c_str());
		return _wtof(buff);
	}

	BOOL GetBOOL(LPCWSTR szSection, LPCWSTR szName, BOOL bDefault = FALSE) const {
		wchar_t buff[64];
		wcscpy_s(buff, 64, bDefault ? L"true" : L"false");
		GetPrivateProfileString(szSection, szName, buff, buff, 64, m_strFile.c_str());
		return !lstrcmpi(buff, L"true") || !lstrcmpi(buff, L"1") || !lstrcmpi(buff, L"y") || !lstrcmpi(buff, L"yes");
	}

	COLORREF GetColour(LPCWSTR szSection, LPCWSTR szName, COLORREF crDefault) const {
		wchar_t buff[1024];
		GetPrivateProfileString(szSection, szName, L"", buff, 1024, m_strFile.c_str());
		int r, g, b;
		if (swscanf_s(buff, L"%d,%d,%d", &r, &g, &b) == 3)
			return RGB(r, g, b);
		return crDefault;
	}

	SIZE GetSize(LPCWSTR szSection, LPCWSTR szName, SIZE sizeDefault) const {
		wchar_t buff[1024];
		GetPrivateProfileString(szSection, szName, L"", buff, 1024, m_strFile.c_str());
		int x, y;
		if (swscanf_s(buff, L"%d,%d", &x, &y) == 2) {
			sizeDefault.cx = x;
			sizeDefault.cy = y;
		}

		return sizeDefault;
	}

	POINT GetPoint(LPCWSTR szSection, LPCWSTR szName, POINT ptDefault) const {
		wchar_t buff[1024];
		GetPrivateProfileString(szSection, szName, L"", buff, 1024, m_strFile.c_str());
		int x, y;
		if (swscanf_s(buff, L"%d,%d", &x, &y) == 2) {
			ptDefault.x = x;
			ptDefault.y = y;
		}

		return ptDefault;
	}

	RECT GetRect(LPCWSTR szSection, LPCWSTR szName, RECT rectDefault) const {
		wchar_t buff[1024];
		GetPrivateProfileString(szSection, szName, L"", buff, 1024, m_strFile.c_str());
		int l, t, r, b;
		if (swscanf_s(buff, L"%d,%d,%d,%d", &l, &t, &r, &b) == 4) {
			rectDefault.left = l;
			rectDefault.top = t;
			rectDefault.right = r;
			rectDefault.bottom = b;
		}

		return rectDefault;
	}

	BOOL GetFile(FileHandler* pFHRet, LPCWSTR szSection, LPCWSTR szName, LPCWSTR szDefault) const {
		wchar_t buff[1024];
		GetPrivateProfileString(szSection, szName, L"", buff, 1024, m_strFile.c_str());
		*pFHRet = *buff ? buff : szDefault;
		return TRUE;
	}

	void SetInt(LPCWSTR szSection, LPCWSTR szName, int nVal) {
		wchar_t buff[64];
		_snwprintf_s(buff, 64, _TRUNCATE, L"%d", nVal);
		SetString(szSection, szName, buff);
	}

	void SetDouble(LPCWSTR szSection, LPCWSTR szName, double dVal) {
		wchar_t buff[64];
		_snwprintf_s(buff, 64, _TRUNCATE, L"%0.15g", dVal);
		SetString(szSection, szName, buff);
	}

	void SetBOOL(LPCWSTR szSection, LPCWSTR szName, BOOL bVal) {
		SetString(szSection, szName, bVal ? L"true" : L"false");
	}

	void SetString(LPCWSTR szSection, LPCWSTR szName, LPCWSTR szValue) {
		WritePrivateProfileString(szSection, szName, szValue, m_strFile.c_str());
	}

	void SetString(LPCWSTR szSection, LPCWSTR szName, LPCSTR szValue) {
		UnicodeMultibyte um(szValue);
		WritePrivateProfileString(szSection, szName, um, m_strFile.c_str());
	}

	void SetColour(LPCWSTR szSection, LPCWSTR szName, COLORREF cr) {
		wchar_t buff[64];
		_snwprintf_s(buff, 64, _TRUNCATE, L"%d,%d,%d", GetRValue(cr), GetGValue(cr), GetBValue(cr));
		SetString(szSection, szName, buff);
	}
};

class JSONFileHandler : public FileHandler
{
public:
	JSONFileHandler(LPCWSTR szFile, LPCWSTR szDefaultExt = NULL) : FileHandler(szFile, szDefaultExt) {}

	bool Load(json::jsonObject& jo, int& line, std::wstring& err) {
		File f;
		if (f.Open(m_strFile.c_str(), GENERIC_READ, OPEN_EXISTING, FILE_SHARE_READ) != ERROR_SUCCESS) {
			line = 0;
			err = L"Can't open file : ";
			err += m_strFile;
			return false;
		}

		DWORD dwLen = f.GetFileSize();
		if (dwLen == 0) {
			line = 0;
			err = L"File is empty : ";
			err += m_strFile;
			return false;
		}

		BYTE* p = new BYTE[dwLen];
		f.Read(p, dwLen);
		bool ok;
		line = 1;
		if ((dwLen >= 2) && (p[0] == 0xff) && (p[1] == 0xfe)) {
			ok = jo.Parse((const wchar_t*)p + 1, line, err);
		}
		else {
			ok = jo.Parse((const wchar_t*)p, line, err);
		}
		delete[] p;
		return ok;
	}

	bool Save(const json::jsonObject& jo) {
		File f;
		if (f.Open(m_strFile.c_str(), GENERIC_WRITE, CREATE_ALWAYS, 0) != ERROR_SUCCESS) {
			return false;
		}

		wchar_t h = 0xfeff;
		f.Write(&h, sizeof(wchar_t));

		std::wstringstream wss;
		jo.GetJSON(wss);
		f.Write(wss.str().c_str(), DWORD(wss.str().length() * sizeof(wchar_t)));
		return true;
	}
};

#define LFH_OPTS_CLEARLOG		0x0001
#define LFH_OPTS_STARTTIME		0x0002

#define LFH_LOG_NONEWLINE		0x0001
#define LFH_LOG_INITIALSPACE	0x0002

class LogFileHandler : public FileHandler
{
public:
	LogFileHandler(int nCSIDL, LPCWSTR szLogName, WORD wOpts = 0, LPCWSTR szFolderName = NULL) : m_bEnabled(TRUE)
	{
		SetSystemPath(nCSIDL, szLogName, L"log", szFolderName);
		if (wOpts & LFH_OPTS_CLEARLOG)
			DeleteFile();	// Start log from scratch

		if (wOpts & LFH_OPTS_STARTTIME)
		{
			DateTime dt;
			dt.SetNow();
			wchar_t buff[256];
			dt.GetDateTime(buff, 256);
			Log(buff);
		}
	}

	void Log(LPCWSTR sz, WORD opts = 0)
	{
		if (!m_bEnabled)	return;

		FILE* fp = wfopen(L"a");
		if (fp)
		{
			if (opts & LFH_LOG_INITIALSPACE)	fwprintf(fp, L" ");
			fwprintf(fp, L"%s", sz);
			if (!(opts & LFH_LOG_NONEWLINE))	fwprintf(fp, L"\n");
			fclose(fp);
		}
	}

	bool Enable(BOOL b) { if (m_bEnabled == b) return false;  m_bEnabled = b;	return true; }
	BOOL GetEnabled() { return m_bEnabled; }

protected:
	BOOL m_bEnabled;
};
