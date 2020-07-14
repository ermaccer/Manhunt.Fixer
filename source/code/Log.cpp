#include "Log.h"
#include <time.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <objbase.h>
#include <iostream>
#include <filesystem>
#include "..\..\framework.h"

#define LOG_FILE L"fixerlog.txt"

FILE* Log::LogFile;
std::wstring strLastPath;
std::wstring strLogPath;

wchar_t log_buff[1024];

void Log::Init()
{
	strLastPath = std::experimental::filesystem::current_path().wstring();
	strLogPath = strLastPath + L"\\" + LOG_FILE;
	LogFile = _wfopen(strLogPath.c_str(), L"a+");
	if (LogFile)
	{
		time_t tt;
		time(&tt);
	    Log:Message(L"Manhunt Fixer Logging Started - %s\n", _wctime(&tt));
	}
	fclose(LogFile);
}

void Log::Clear()
{
	LogFile = _wfopen(LOG_FILE, L"w");
	fclose(LogFile);
}

void Log::Message(const wchar_t * fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	vswprintf_s(log_buff, 1024, fmt, va);
	va_end(va);

	LogFile = _wfopen(strLogPath.c_str(), L"a+");
	fwprintf(LogFile, L"%s", log_buff);
	fclose(LogFile);
    //printf(log_buff);

}

void Log::ShowLog()
{
	ShellExecute(0, 0, strLogPath.c_str(), 0, 0, SW_SHOW);
}
