#pragma once
#include <string>
#include <Windows.h>
#define TOOL_NAME L"Manhunt Fixer"


std::wstring   SetPathFromButton(wchar_t* filter, wchar_t* ext, HWND hWnd);
std::wstring   SetSavePathFromButton(wchar_t* filter, wchar_t* ext, HWND hWnd);
std::wstring   SetFolderFromButton(HWND hWnd);

bool           ValidatePath(std::wstring path);


extern HWND GlobalHWND;