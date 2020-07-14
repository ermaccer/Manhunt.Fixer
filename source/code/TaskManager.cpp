#include "TaskManager.h"
#include <filesystem>
#include <CommCtrl.h>
#include <shlobj.h>
#include <fstream>
#include <vector>

#include "..\unzipper.h"
#include "Error.h"
#include "GameData.h"
#include "FileDownload.h"
#include "Log.h"
#include "Common.h"


using namespace zipper;

HWND* hProgressBar = nullptr;
HWND* hTaskName = nullptr;
std::wstring strGamePath;

struct DownloadPair {
	std::wstring url;
	std::wstring name;
};


std::vector<DownloadPair> vDownloads;



std::wstring strCheckFiles[] = {
	L"scripts\\Manhunt.WidescreenFix.asi",
	L"scripts\\Manhunt.WidescreenFix.ini",
	L"d3d8.dll",
	L"d3d8.ini",
	L"dinput8.dll",
	L"scripts\\AudioFix.asi",
	// in case they are in root
	L"AudioFix.asi",
	L"Manhunt.WidescreenFix.asi",
	L"Manhunt.WidescreenFix.ini",
};


void TaskManager::InitItemsToDownload()
{
	DownloadPair downloadData[3] = {
	L"https://github.com/ThirteenAG/WidescreenFixesPack/releases/download/manhunt/Manhunt.WidescreenFix.zip", L"Manhunt.WidescreenFix.zip",
	L"https://github.com/ThirteenAG/d3d8-wrapper/releases/latest/download/d3d8.zip", L"d3d8.zip",
	L"https://github.com/ermaccer/Manhunt.AudioFix/releases/latest/download/AudioFix.zip",L"AudioFix.zip"
	};

	for (int i = 0; i < 3; i++)
		vDownloads.push_back(downloadData[i]);

}

void TaskManager::ClearDownloads()
{
	vDownloads.clear();
	InitItemsToDownload();
}

void TaskManager::AddDownload(std::wstring url, std::wstring file)
{
	DownloadPair dl = { url,file };
	vDownloads.push_back(dl);
}

void TaskManager::LinkItems(HWND * bar, HWND * tname, std::wstring gamePath)
{
	hProgressBar = bar;
	hTaskName = tname;
	strGamePath = gamePath;
	SendMessage(*hProgressBar, PBM_SETSTEP, 1, 0);
	SendMessage(*hProgressBar, PBM_SETPOS, 0, 0);
	SendMessage(*hProgressBar, PBM_SETSTATE, PBST_NORMAL, 0);
	SendMessage(*hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 6));
}

void TaskManager::TaskSetName(std::wstring name)
{
	SetWindowText(*hTaskName, name.c_str());
}

void TaskManager::TaskBegin(std::wstring name)
{
	SendMessage(*hProgressBar, PBM_SETSTATE, PBST_NORMAL, 0);
	std::experimental::filesystem::current_path(strGamePath);
	TaskSetName(name);
}

bool TaskManager::TaskEnd(bool * val)
{
	if (*val)
	{
		SendMessage(*hProgressBar, PBM_STEPIT, 0, 0);
		TaskSetName(L"Idle");
	}
	else
	{
		SendMessage(*hProgressBar, PBM_SETSTATE, PBST_ERROR, 0);
		TaskSetName(L"Task Failed");
		PushErrorMessage();
	}
	return *val;
}

bool TaskManager::TaskVerifyGameData()
{
	TaskBegin(L"Veryfing Game Data");
	bool bResult = VerifyManhuntGameData();
	return TaskEnd(&bResult);
}

bool TaskManager::TaskDownloadRequiredFiles()
{
	if (!std::experimental::filesystem::exists(L"temp"))
		std::experimental::filesystem::create_directory(L"temp");

	std::experimental::filesystem::current_path(L"temp");

	bool bResult = true;
	for (int i = 0; i < vDownloads.size(); i++)
	{
		bResult = RequestDownloadFile(vDownloads[i].url.c_str(), vDownloads[i].name.c_str());
	}
	return TaskEnd(&bResult);
}

bool TaskManager::TaskUnzipDownloadedFiles()
{
	TaskBegin(L"Decompressing Archives");
	std::experimental::filesystem::current_path(L"temp");
	bool bResult = true;
	for (int i = 0; i < vDownloads.size(); i++)
	{
		std::wstring file = vDownloads[i].name;
		std::string tmp("", file.length());
		std::copy(file.begin(), file.end(), tmp.begin());
		Unzipper zip(tmp);
		bResult  = zip.extract("..");
		if (!bResult)
			break;
		if (bResult)
			zip.close();

		_wremove(vDownloads[i].name.c_str());
		Log::Message(L"INFO: %s | %s %s\n", L"TaskUnzipDownloadedFiles", L"Decompressing: ", vDownloads[i].name.c_str());
	}
	// move frontendtextures
	std::experimental::filesystem::current_path(L"..");
	
	if (std::experimental::filesystem::exists("Manhunt.WidescreenFrontend"))
	{
		if (std::experimental::filesystem::is_directory("Manhunt.WidescreenFrontend"))
		{
			std::experimental::filesystem::current_path(L"Manhunt.WidescreenFrontend");

			for (const auto & file : std::experimental::filesystem::recursive_directory_iterator(std::experimental::filesystem::current_path()))
			{
				size_t len = strGamePath.length() + wcslen(L"\\") + wcslen(L"Manhunt.WidescreenFrontend");
				std::wstring oldPath = file.path().wstring().substr(len + 1,file.path().wstring().length() - len);
				std::wstring newPath = L"..\\" + oldPath;
				std::experimental::filesystem::copy(oldPath,newPath,std::experimental::filesystem::copy_options::overwrite_existing);
				_wremove(oldPath.c_str());
			}
		}

	}


	return TaskEnd(&bResult);
}

bool TaskManager::TaskPossiblyDeleteExistingFiles()
{
	bool bResult = true;
	TaskBegin(L"Checking for existing files");

	for (int i = 0; i < 9; i++)
	{
		if (std::experimental::filesystem::exists(strCheckFiles[i]))
		{
			Log::Message(L"INFO: %s | %s %s\n", L"TaskPossiblyDeleteExistingFiles", L"Remove: ", strCheckFiles[i].c_str());
			_wremove(strCheckFiles[i].c_str());
		}

	}
	return TaskEnd(&bResult);
}

bool TaskManager::TaskPatchExecutable()
{
	bool bResult = true;
	WORD dwFlags = 0;
	DWORD dwOffset = 0x8DE;

	TaskBegin(L"Patching executable");
	if (std::experimental::filesystem::exists(L"testapp.exe"))
	{
		std::experimental::filesystem::rename(L"manhunt.exe", L"manhunt.exe.backup");
		std::experimental::filesystem::rename(L"testapp.exe", L"manhunt.exe");
		Log::Message(L"INFO: %s | %s %s %s\n", L"TaskPatchExecutable", L"A copy of previous executable has been saved as", L"manhunt.exe.backup",L"You may remove it on your own");
	}
	std::ifstream pReadExecutable(L"manhunt.exe", std::ifstream::binary);
	pReadExecutable.seekg(dwOffset, std::ifstream::beg);
	pReadExecutable.read((char*)&dwFlags, sizeof(WORD));
	pReadExecutable.close();

	if (!(dwFlags & (IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE | IMAGE_DLLCHARACTERISTICS_NX_COMPAT)))
	{
		Log::Message(L"INFO: %s | %s\n", L"TaskPatchExecutable", L"Executable is already patched.");
	}
	else
	{
		dwFlags &= ~(IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE | IMAGE_DLLCHARACTERISTICS_NX_COMPAT);
		std::ofstream pExecutable(L"manhunt.exe", std::wofstream::binary | std::wofstream::in | std::wofstream::out);
		pExecutable.seekp(dwOffset, std::ofstream::beg);
		pExecutable.write((char*)&dwFlags, sizeof(WORD));
		pExecutable.close();

	}


	
	return TaskEnd(&bResult);
}

bool TaskManager::TaskApply60FPSLimit()
{
	bool bResult = true;
	TaskBegin(L"Configuring");
	if (std::experimental::filesystem::exists(L"temp"))
	  std::experimental::filesystem::remove_all(L"temp");
	if (std::experimental::filesystem::exists(L"Manhunt.WidescreenFrontend"))
		std::experimental::filesystem::remove_all(L"Manhunt.WidescreenFrontend");

	if (std::experimental::filesystem::exists(L"d3d8.ini"))
	{
		std::wstring path = std::experimental::filesystem::current_path().wstring() + L"\\d3d8.ini";
		WritePrivateProfileString(L"MAIN", L"FPSLimit", L"60", path.c_str());

		Log::Message(L"INFO: %s | %s\n", L"TaskApply60FPSLimit", L"Configuration set");
	}
	else
	{
		Log::Message(L"ERROR: %s | %s\n", L"TaskApply60FPSLimit", L"Could not set configuration (d3d8.ini missing)");
		bResult = false;
	}

	return TaskEnd(&bResult);
}

void TaskManager::TaskComplete()
{
	SendMessage(*hProgressBar, PBM_SETPOS,6, 0);
}

void AskForPMHSettings()
{

	if (MessageBox(GlobalHWND, L"PluginMH has been installed. Do you wish to open configuration file? \n\nPlease remember that if any update is available, you need to update it manually", TOOL_NAME, MB_ICONINFORMATION | MB_YESNO) == IDYES)
	{
		std::experimental::filesystem::current_path(strGamePath);
		ShellExecute(0, 0, L"PluginMH.ini", 0, 0, SW_SHOW);
	}

}
