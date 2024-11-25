#pragma once

#include <wrap32lib.h>

#include <StringParser.h>
#include "File.h"
#include "FileHandler.h"
#include <Registry.h>
#include "Directory.h"
#include <json.h>

class AppRegistry
{
public:
	AppRegistry(LPCWSTR appname, bool create = false) : m_optionsPath(L""), m_installPath(L"")
	{
		SetAppName(appname, create);

		// Get Install Path

		// Try registry under HKLM\SOFTWARE\app\Path
		std::wstring regpath = L"SOFTWARE\\";
		regpath += appname;

		RegistryKey rk(HKEY_LOCAL_MACHINE, regpath.c_str());
		std::wstring installPath;
		if (rk.ReadString(L"Path", installPath)) {
			m_installPath = installPath.c_str();
		}
		else {
			// Use the running module path
			wchar_t buff[MAX_PATH + 1];
			::GetModuleFileName(NULL, buff, MAX_PATH + 1);
			m_installPath = buff;
			m_installPath.RemoveSubFolder();
		}
	}

	bool SetAppName(LPCWSTR appname, bool create = false) {
		m_appname = appname;

		PWSTR path;
		if (SHGetKnownFolderPath(FOLDERID_ProgramData, 0, NULL, &path) == ERROR_SUCCESS) {
			m_optionsPath = path;
			m_optionsPath.AddSubFolder(appname);
			CoTaskMemFree(path);
		}
		else {
			m_optionsPath = L"C:\\ProgramData";
			m_optionsPath.AddSubFolder(appname);
		}

		if (create)
			m_optionsPath.Create();

		return m_optionsPath.Exists();
	}
/*
	BOOL ImportFile(LPCWSTR filepath) {
		wchar_t filename[256];
		wchar_t ext[256];
		_wsplitpath_s(filepath, NULL, 0, NULL, 0, filename, 256, ext, 256);
		std::wstring fileext = filename;
		fileext += ext;
		std::wstring dest = GetFullPath(NULL, fileext.c_str());
		return CopyFile(filepath, dest.c_str(), TRUE);
	}

	void GetLogPath(std::wstring& logPath, LPCWSTR logPrefix = L"log") const {
		Directory d = m_optionsPath;
		d.AddSubFolder(m_system.c_str());
		d.AddSubFolder(L"Logs");
		d.Create();
		d.AddSubFolder(std::to_wstring(m_id).c_str());
		d.Create();
		d.AddSubFolder(logPrefix);

		logPath = d;
	}
	*/

	std::wstring GetOptionsPath(LPCWSTR subFolder, bool create, LPCWSTR file = NULL) const {
		Directory d = m_optionsPath;
		if (subFolder) {
			d.AddSubFolder(subFolder);
			if (create && !d.Exists())
				d.Create();
		}

		if (file)
			d.AddSubFolder(file);

		return std::wstring(d);
	}
	
	std::wstring GetInstallPath(LPCWSTR subFolder, LPCWSTR file = NULL) const {
		Directory d = m_installPath;
		if (subFolder)
			d.AddSubFolder(subFolder);

		if (file)
			d.AddSubFolder(file);

		return std::wstring(d);
	}

	bool JSONLoadOptionsFile(json::jsonObject& jo, int& line, std::wstring& err, LPCWSTR filename, LPCWSTR subFolder = NULL) const {
		Directory d = m_optionsPath;
		if (subFolder)
			d.AddSubFolder(subFolder);
		d.AddSubFolder(filename);

		JSONFileHandler jfh(d);
		return jfh.Load(jo, line, err);
	}

/*
	std::wstring GetRuntimePath(LPCWSTR subFolder, LPCWSTR filename = NULL) const {
		Directory d = m_optionsPath;
		d.AddSubFolder(m_system.c_str());
		if (subFolder)
			d.AddSubFolder(subFolder);
		if (filename)
			d.AddSubFolder(filename);

		return std::wstring(d);
	}

	File DSROpenInstanceFile(LPCWSTR fileName, bool write = false) const {
		std::wstring fullPath;
		GetInstanceFolder(fullPath);
		fullPath += L"\\";
		fullPath += fileName;

		if (write)
			return File(fullPath.c_str(), GENERIC_WRITE, CREATE_ALWAYS);
		return File(fullPath.c_str(), GENERIC_READ, OPEN_EXISTING, FILE_SHARE_READ);
	}

	bool DSRLoadJSONRuntimeFile(json::jsonObject& jo, int& line, std::wstring& err, LPCWSTR filename, LPCWSTR subFolder = NULL) const {
		Directory d = m_optionsPath;
		d.AddSubFolder(m_system.c_str());
		if (subFolder)
			d.AddSubFolder(subFolder);
		d.AddSubFolder(filename);

		JSONFileHandler jfh(d);
		return jfh.Load(jo, line, err);
	}

	bool DSRLoadJSONInstanceFile(json::jsonObject& jo, int& line, std::wstring& err, LPCWSTR filename, LPCWSTR subFolder = NULL) const {
		std::wstring fullPath;
		GetInstanceFolder(fullPath, subFolder);
		fullPath += L"\\";
		fullPath += filename;

		JSONFileHandler jfh(fullPath.c_str());
		return jfh.Load(jo, line, err);
	}

	bool DSRLoadJSONInstallFile(json::jsonObject& jo, int& line, std::wstring& err, LPCWSTR filename, LPCWSTR subFolder = NULL) const {
		std::wstring fullPath = GetInstallPath(subFolder, filename);
		JSONFileHandler jfh(fullPath.c_str());
		return jfh.Load(jo, line, err);
	}

	bool DSRSaveJSONFile(LPCWSTR filename, json::jsonBuilder& jb) {
		File f = DSROpenInstanceFile(filename, true);
		if (!f.IsOpen())
			return false;

		f.Write(jb.str().c_str());
		return true;
	}

protected:
	void GetInstanceFolder(std::wstring& ws, LPCWSTR subFolder = NULL)  const {
		Directory d = m_optionsPath;
		d.AddSubFolder(m_system.c_str());
		d.AddSubFolder(L"Instances");
		d.Create();
		d.AddSubFolder(std::to_wstring(m_id).c_str());
		d.Create();

		if (subFolder) {
			d.AddSubFolder(subFolder);
			d.Create();
		}

		ws = d;
	}

	File DSROpenFile(LPCWSTR relPath, bool write = false) const {
		Directory d = m_optionsPath;
		d.AddSubFolder(m_system.c_str());
		d.AddSubFolder(relPath);
		if (write)
			return File(d, GENERIC_WRITE, CREATE_ALWAYS);

		return File(d, GENERIC_READ, OPEN_EXISTING, FILE_SHARE_READ);
	}
	*/
protected:
	std::wstring m_appname;		// e.g. myapp

	Directory m_installPath;	// e.g. c:\program files\myapp
	Directory m_optionsPath;	// e.g. c:\programdata\myapp
};

