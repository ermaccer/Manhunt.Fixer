#include "Common.h"
#include <CommCtrl.h>
#include <shlobj.h>
#include <filesystem>
#include <fstream>

#include "Log.h"

std::wstring SetPathFromButton(wchar_t* filter, wchar_t* ext, HWND hWnd)
{
	wchar_t szBuffer[MAX_PATH] = {};
	OPENFILENAME ofn = {};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = szBuffer;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = ext;
	std::wstring out;
	if (GetOpenFileName(&ofn))
		out = szBuffer;

	return out;
}


std::wstring  SetSavePathFromButton(wchar_t* filter, wchar_t* ext, HWND hWnd)
{
	wchar_t szBuffer[MAX_PATH] = {};
	OPENFILENAME ofn = {};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = szBuffer;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = ext;
	std::wstring out;
	if (GetSaveFileName(&ofn))
		out = szBuffer;

	return out;
}

std::wstring   SetFolderFromButton(HWND hWnd)
{
	wchar_t szBuffer[MAX_PATH];

	BROWSEINFO bi = {};
	bi.lpszTitle = L"Select Manhunt directory";
	bi.hwndOwner = hWnd;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	LPITEMIDLIST idl = SHBrowseForFolder(&bi);

	std::wstring out;

	if (idl)
	{
		SHGetPathFromIDList(idl, szBuffer);
		out = szBuffer;

	}

	return out;
}

bool ValidatePath(std::wstring path)
{
	if (!std::filesystem::exists(path) && !std::filesystem::is_directory(path))
	{
		MessageBox(GlobalHWND, L"Invalid path!", TOOL_NAME, MB_ICONINFORMATION);
		return false;
	}
	std::filesystem::current_path(path);
	Log::Message(L"INFO: %s | %s %s\n", L"ValidatePath", L"Attempting to validate path:", path.c_str());
	// check one: manhunt.exe

	if (!std::filesystem::exists(L"manhunt.exe"))
	{
		MessageBox(GlobalHWND, L"Could not find manhunt.exe in this directory.", TOOL_NAME, MB_ICONINFORMATION);
		return false;
	}


	// check two: write permission
	// should improve one day
	std::wofstream oFile(L"acfgzx@4@@5l.4ia", std::ofstream::binary);
	if (!oFile.good())
	{
		MessageBox(GlobalHWND, L"You do not have permission to write in this directory, try running as admin.", TOOL_NAME, MB_ICONINFORMATION);
		return false;
	}


	oFile.close();
	std::filesystem::remove(L"acfgzx@4@@5l.4ia");
	return true;
}

