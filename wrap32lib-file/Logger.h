#pragma once

#include <vector>
#include <map>
#include <string>
#include <stdint.h>
#include <CriticalSection.h>

enum logPriority {
	critical,
	error,
	warning,
	info,
	debug
};

class Logger
{
protected:
	enum destination {
		file,
		console,
		debugstring,
		popup
	};

	class LoggerPath {
	public:
		logPriority m_minPriority;
		destination m_dest;
		std::wstring m_name;
	};

public:
	Logger()	{
		for (int i = debug; i >= 0; i--)
			m_actives[i] = false;
	}

	// Indicate source logs of priority >= prio be added to a file.
	void AddFilePath(const std::wstring& name, logPriority prio = error) {
		for (int i = prio; i >= 0; i--)
			m_actives[i] = true;

		m_paths.push_back({prio, file, name});
		LogToFile(name, L"START", L"-------- Logging session started --------");
	}

	// Indicate source logs of priority >= prio be output to the console
	void AddConsole(logPriority prio = error) {
		for (int i = prio; i >= 0; i--)
			m_actives[i] = true;

		m_paths.push_back({prio, console, L""});
	}

	void AddDebug(logPriority prio = error) {
		for (int i = prio; i >= 0; i--)
			m_actives[i] = true;

		m_paths.push_back({prio, debugstring, L""});
	}

	void AddPopup(logPriority prio = error) {
		m_paths.push_back({ prio, popup, L"" });
	}

	// Log a message, specifying the source(es) and priority.
	void Log(const wchar_t* classname, logPriority prio, const wchar_t* fmt, ...) {
		if (!IsLogging(prio))
			return;

		va_list vlargs;
		va_start(vlargs, fmt);
		LogVA(classname, prio, fmt, vlargs);
		va_end(vlargs);
	}

	void Log(logPriority prio, const wchar_t* fmt, ...) {
		if (!IsLogging(prio))
			return;

		va_list vlargs;
		va_start(vlargs, fmt);
		LogVA(NULL, prio, fmt, vlargs);
		va_end(vlargs);
	}

	void LogVA(const wchar_t* classname, logPriority prio, const wchar_t* fmt, va_list vlargs);
	void LogVAa(const char* classname, logPriority prio, const char* fmt, va_list vlargs);

	bool IsLogging(logPriority prio) {
		return m_actives[prio];
	}

protected:
	void LogToFile(const std::wstring& fileName, const wchar_t* prio, const wchar_t* message);
	void LogToConsole(const wchar_t* prio, const wchar_t* message);
	void LogToDebug(const wchar_t* classname, const wchar_t* prio, const wchar_t* message);
	void LogToPopup(const wchar_t* prio, const wchar_t* message);

protected:
	std::vector<LoggerPath> m_paths;	// the routing table
	bool m_actives[5];	// if debug active on these
	CriticalSection m_cs;
};
