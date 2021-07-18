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
	L"ddraw.dll",
	L"scripts\\AudioFix.asi",
	L"scripts\\HalosFix.asi",
	L"scripts\\DiscordPlugin.asi",
	// in case they are in root
	L"AudioFix.asi",
	L"Manhunt.WidescreenFix.asi",
	L"Manhunt.WidescreenFix.ini",
	L"HalosFix.asi", // no longer needed, but remove because of incompatibilty with mhp (both doing same thing)
	L"DiscordPlugin.asi",
	L"MHP.ini",
	L"MHP.asi",
	L"scripts\\MHP.ini",
	L"scripts\\MHP.asi",
	// backups
	// blood fix 
	L"pictures\\frontend_pc.txd.bak",
	// ps2 cash
	L"levels\\global\\charpak\\cash_pc.bak"
	// models fix
	L"levels\\global\\pak\\gmodelspc.bak"
	// halos fix
	L"pictures\\halofix.txd",
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
	std::filesystem::current_path(strGamePath);
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
	if (!std::filesystem::exists(L"temp"))
		std::filesystem::create_directory(L"temp");

	std::filesystem::current_path(L"temp");

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
	std::filesystem::current_path(L"temp");
	bool bResult = true;
	for (int i = 0; i < vDownloads.size(); i++)
	{
		if (std::filesystem::exists(vDownloads[i].name))
		{
			std::wstring file = vDownloads[i].name;
			std::string tmp("", file.length());
			std::copy(file.begin(), file.end(), tmp.begin());
			Unzipper zip(tmp);
			bResult = zip.extract("..");
			if (!bResult)
				break;
			if (bResult)
				zip.close();

			_wremove(vDownloads[i].name.c_str());


			Log::Message(L"INFO: %s | %s %s\n", L"TaskUnzipDownloadedFiles", L"Decompressing: ", vDownloads[i].name.c_str());
		}
		else
		{
			bResult = false;
			Log::Message(L"INFO: %s | %s %s\n", L"TaskUnzipDownloadedFiles", L"Failed to decompress: ", vDownloads[i].name.c_str());
			break;
		}


	}


	std::filesystem::current_path(L"..");

	// if scripts still doesnt exist somehow

	if (!std::filesystem::exists("scripts"))
	{
		std::filesystem::create_directory("scripts");
	}

	// move audiofix
	if (std::filesystem::exists("AudioFix.asi"))
	{
		std::filesystem::rename("AudioFix.asi", "scripts\\AudioFix.asi");
	}


	// move mhp
	if (std::filesystem::exists("MHP.asi"))
	{
		std::filesystem::rename("MHP.asi", "scripts\\MHP.asi");
	}
	if (std::filesystem::exists("MHP.ini"))
	{
		std::filesystem::rename("MHP.ini", "scripts\\MHP.ini");
	}

	// rename asi loader
	if (std::filesystem::exists("dinput8.dll"))
	{
		std::filesystem::rename("dinput8.dll", "ddraw.dll");
	}

	// blood
	if (std::filesystem::exists("bloodfix.txd"))
	{
		if (std::filesystem::exists("pictures\\frontend_pc.txd"))
		{
			std::filesystem::rename("pictures\\frontend_pc.txd", "pictures\\frontend_pc.txd.bak");
			Log::Message(L"INFO: %s | %s %s %s\n", L"TaskUnzipDownloadedFiles", L"A copy of previous frontend_pc.txd has been saved as", L"frontend_pc.txd.bak", L"You may remove it on your own");
		}
		std::filesystem::rename("bloodfix.txd", "pictures\\frontend_pc.txd");
	}

	// ps2 cash
	if (std::filesystem::exists("ps2cash.txd"))
	{
		if (std::filesystem::exists("levels\\global\\charpak\\cash_pc.txd"))
		{
			std::filesystem::rename("levels\\global\\charpak\\cash_pc.txd", "levels\\global\\charpak\\cash_pc.bak");
			Log::Message(L"INFO: %s | %s %s %s\n", L"TaskUnzipDownloadedFiles", L"A copy of previous cash_pc.txd has been saved as", L"cash_pc.bak", L"You may remove it on your own");
		}
		std::filesystem::rename("ps2cash.txd", "levels\\global\\charpak\\cash_pc.txd");
	}


	// models fix
	if (std::filesystem::exists("gmodels_fix.dff"))
	{
		if (std::filesystem::exists("levels\\global\\pak\\gmodelspc.dff"))
		{
			std::filesystem::rename("levels\\global\\pak\\gmodelspc.dff", "levels\\global\\pak\\gmodelspc.bak");
			Log::Message(L"INFO: %s | %s %s %s\n", L"TaskUnzipDownloadedFiles", L"A copy of previous gmodelspc.dff has been saved as", L"gmodelspc.bak", L"You may remove it on your own");
		}
		std::filesystem::rename("gmodels_fix.dff", "levels\\global\\pak\\gmodelspc.dff");
	}

	if (std::filesystem::exists("gmodels_fix.txd"))
	{
		if (std::filesystem::exists("levels\\global\\pak\\gmodelspc.txd"))
		{
			std::filesystem::rename("levels\\global\\pak\\gmodelspc.txd", "levels\\global\\pak\\gmodelspc.txd.bak");
			Log::Message(L"INFO: %s | %s %s %s\n", L"TaskUnzipDownloadedFiles", L"A copy of previous gmodelspc.txd has been saved as", L"gmodelspc.txd.bak", L"You may remove it on your own");
		}
		std::filesystem::rename("gmodels_fix.txd", "levels\\global\\pak\\gmodelspc.txd");
	}
	
	// move discord plugin
	if (std::filesystem::exists("DiscordPlugin.asi"))
	{
		std::filesystem::rename("DiscordPlugin.asi", "scripts\\DiscordPlugin.asi");
	}
	// move wide screen textures
	if (std::filesystem::exists("Manhunt.WidescreenFrontend"))
	{
		if (std::filesystem::is_directory("Manhunt.WidescreenFrontend"))
		{
			std::filesystem::current_path(L"Manhunt.WidescreenFrontend");

			for (const auto & file : std::filesystem::recursive_directory_iterator(std::filesystem::current_path()))
			{
				size_t len = strGamePath.length() + wcslen(L"\\") + wcslen(L"Manhunt.WidescreenFrontend");
				std::wstring oldPath = file.path().wstring().substr(len + 1, file.path().wstring().length() - len);
				std::wstring newPath = L"..\\" + oldPath;
				std::filesystem::copy(oldPath, newPath, std::filesystem::copy_options::overwrite_existing);
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

	for (int i = 0; i < sizeof(strCheckFiles[0]) / sizeof(strCheckFiles); i++)
	{
		if (std::filesystem::exists(strCheckFiles[i]))
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
	if (std::filesystem::exists(L"testapp.exe"))
	{
		std::filesystem::rename(L"manhunt.exe", L"manhunt.exe.backup");
		std::filesystem::rename(L"testapp.exe", L"manhunt.exe");
		Log::Message(L"INFO: %s | %s %s %s\n", L"TaskPatchExecutable", L"A copy of previous executable has been saved as", L"manhunt.exe.backup", L"You may remove it on your own");
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
	if (std::filesystem::exists(L"temp"))
		std::filesystem::remove_all(L"temp");
	if (std::filesystem::exists(L"Manhunt.WidescreenFrontend"))
		std::filesystem::remove_all(L"Manhunt.WidescreenFrontend");

	if (std::filesystem::exists(L"d3d8.ini"))
	{
		std::wstring path = std::filesystem::current_path().wstring() + L"\\d3d8.ini";
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
	SendMessage(*hProgressBar, PBM_SETPOS, 6, 0);
}

void AskForPMHSettings()
{

	if (MessageBox(GlobalHWND, L"PluginMH has been installed. Do you wish to open configuration file? \n\nPlease remember that if any update is available, you need to update it manually", TOOL_NAME, MB_ICONINFORMATION | MB_YESNO) == IDYES)
	{
		std::filesystem::current_path(strGamePath);
		ShellExecute(0, 0, L"PluginMH.ini", 0, 0, SW_SHOW);
	}

}
