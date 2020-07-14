#pragma once
#include <iostream>

class Log {
public:
	static FILE* LogFile;
	static void Init();
	static void Clear();
	static void Message(const wchar_t* fmt, ...);
	static void ShowLog();
};