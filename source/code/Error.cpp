#include "Error.h"
#include "Log.h"
#include "Common.h"
#include <Windows.h>


void PushErrorMessage(eErrorType type)
{
	wchar_t message[512] = {};
	DWORD dwWarning = 0;
	switch (type)
	{
	case ERROR_BAD:
		wsprintf(message, L"An error has occured, would you like to view log?");
		dwWarning = MB_ICONWARNING;
		break;
	case ERROR_COMPLETE:
		wsprintf(message, L"Operation complete! Would you like to view log?");
		dwWarning = MB_ICONINFORMATION;
		break;
	}
	if (MessageBox(GlobalHWND, message, TOOL_NAME, dwWarning | MB_YESNO) == IDYES)
	{
		Log::ShowLog();
	}

}

std::wstring GetLastErrorAsString(DWORD dwError)
{
	if (dwError == 0)
		return std::wstring();

	LPWSTR buffer = nullptr;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&buffer, 0, NULL);

	std::wstring message(buffer, wcslen(buffer));


	return message;
}