#pragma once
#include <string>
#include <Windows.h>

enum eErrorType {
	ERROR_BAD,
	ERROR_COMPLETE
};

void PushErrorMessage(eErrorType type = ERROR_BAD);
std::wstring GetLastErrorAsString(DWORD dwError);