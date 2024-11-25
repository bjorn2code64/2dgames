
#include "wrap32lib.h"
#include "Logger.h"
#include "File.h"
#include "unicodemultibyte.h"

//#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>

///////////////////////////////////////////////////////////////////////

void Logger::LogVA(const wchar_t* classname, logPriority prio, const wchar_t* fmt, va_list vlargs)
{
	// Build up the list of destinations, making sure there are no duplicates
	std::vector<std::wstring> files;
	bool bConsole = false;
	bool bDebug = false;
	bool bPopup = false;

	for (auto& path : m_paths)
	{
		if (prio <= path.m_minPriority)
		{
			switch (path.m_dest)
			{
			case file:
				if (std::find(files.begin(), files.end(), path.m_name) == files.end())
					files.push_back(path.m_name);
				break;

			case debugstring:
				bDebug = true;
				break;

			case console:
				bConsole = true;
				break;

			case popup:
				bPopup = true;
				break;
			}
		}
	}

	// Build the message to be logged
	wchar_t buff[4096];
//	va_list ap;
//#pragma warning( push )
//#pragma warning( disable : 4996 )
//	va_start(ap, fmt);
	std::vswprintf(&buff[0], 4096, fmt, vlargs);
//	va_end(ap);
//#pragma warning( pop )

	// Get the text for the priority
	static const wchar_t* priorities[] = {
		L"CRITICAL",
		L"ERROR",
		L"WARNING",
		L"INFO",
		L"DEBUG"
	};

	const wchar_t* priority_string = (prio >= 0) && (prio <= 4) ? priorities[prio] : NULL;

	// Log to the marked destinations
	for (auto& file : files)
		LogToFile(file, priority_string, buff);

	if (bDebug)
		LogToDebug(classname, priority_string, buff);

	if (bConsole)
		LogToConsole(priority_string, buff);

	if (bPopup)
		LogToPopup(priority_string, buff);
}

void Logger::LogVAa(const char* classname, logPriority prio, const char* fmt, va_list vlargs)
{
	// Build up the list of destinations, making sure there are no duplicates
	std::vector<std::wstring> files;
	bool bConsole = false;
	bool bDebug = false;
	bool bPopup = false;

	for (auto& path : m_paths)
	{
		if (prio <= path.m_minPriority)
		{
			switch (path.m_dest)
			{
			case file:
				if (std::find(files.begin(), files.end(), path.m_name) == files.end())
					files.push_back(path.m_name);
				break;

			case debugstring:
				bDebug = true;
				break;

			case console:
				bConsole = true;
				break;

			case popup:
				bPopup = true;
				break;
			}
		}
	}

	// Build the message to be logged
	char buff[4096];
	//	va_list ap;
	//#pragma warning( push )
	//#pragma warning( disable : 4996 )
	//	va_start(ap, fmt);
	vsprintf_s(&buff[0], 4096, fmt, vlargs);
	//	va_end(ap);
	//#pragma warning( pop )

		// Get the text for the priority
	static const char* priorities[] = {
		"CRITICAL",
		"ERROR",
		"WARNING",
		"INFO",
		"DEBUG"
	};

	const char* priority_string = (prio >= 0) && (prio <= 4) ? priorities[prio] : NULL;

	// Log to the marked destinations
	for (auto& file : files)
		LogToFile(file, UnicodeMultibyte(priority_string), UnicodeMultibyte(buff));

	if (bDebug)
		LogToDebug(classname ? (const wchar_t*)UnicodeMultibyte(classname) : NULL, UnicodeMultibyte(priority_string), UnicodeMultibyte(buff));

	if (bConsole)
		LogToConsole(UnicodeMultibyte(priority_string), UnicodeMultibyte(buff));

	if (bPopup)
		LogToPopup(UnicodeMultibyte(priority_string), UnicodeMultibyte(buff));
}

///////////////////////////////////////////////////////////////////////

void Logger::LogToFile(const std::wstring& name, const wchar_t* prio, const wchar_t* message)
{
	// get time and date
	time_t result = time(nullptr);
	tm tm;
	localtime_s(&tm, &result);

	// Build the filename based on the name, the date and a .log extension
	std::wostringstream ss;
	ss << name << std::put_time(&tm, L"-%Y-%m-%d.log");

	// Open the file for append
	File f;
	m_cs.Enter();
	if (f.Open(ss.str().c_str(), FILE_APPEND_DATA, OPEN_EXISTING, FILE_SHARE_READ) != ERROR_SUCCESS) {
		if (f.Open(ss.str().c_str(), FILE_APPEND_DATA, OPEN_ALWAYS, FILE_SHARE_READ) != ERROR_SUCCESS) {
		return;
		}
		// just created. Write the BOM
		f.WriteBOM();
	}

	// Log the time only (date is in the filename), the priority if it's !NULL, and the message
	std::wostringstream wodata;
	wodata << std::put_time(&tm, L"%H:%M:%S: ");
	if (prio)
		wodata << prio << L". ";
	wodata << message << '\n';
	f.Write(wodata.str().c_str());
	f.Close();
	m_cs.Leave();
}

///////////////////////////////////////////////////////////////////////

void Logger::LogToConsole(const wchar_t* prio, const wchar_t* message)
{
	if (prio)
		wprintf(L"%s: %s\n", prio, message);
	else
		wprintf(L"%s\n", message);
}

void Logger::LogToPopup(const wchar_t* prio, const wchar_t* message)
{
	MessageBox(NULL, message, prio, MB_OK);
}

///////////////////////////////////////////////////////////////////////

void Logger::LogToDebug(const wchar_t* classname, const wchar_t* prio, const wchar_t* message)
{
	std::wstringstream wss;
	if (prio) {
		wss << prio << L": ";
	}

	if (classname) {
		wss << classname << L":";
	}

	wss << message << L"\n";
	OutputDebugString(wss.str().c_str());
}

///////////////////////////////////////////////////////////////////////
