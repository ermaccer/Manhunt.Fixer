#include "SteamDirectory.h"
#include "Error.h"
#include "Log.h"
#include "..\framework.h"
#include <vector>
#include <filesystem>
#include <fstream>

#include "vdf_parser.hpp"

#define MANHUNT_STEAM_ID 12130

std::vector<std::wstring> vLibraryPaths;
std::wstring strManhuntPath; bool bIsManhuntPathAvailable = false;

std::wstring FindSteamDirectory()
{
	wchar_t szPath[MAX_PATH] = {};
	DWORD dwPathSize = sizeof(szPath), dwType = REG_SZ;
	HKEY hKey = 0;

	if (!(RegOpenKey(HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam", &hKey) == ERROR_SUCCESS))
	{

		Log::Message(L"ERROR: %s | %s (%d)\n",L"RegOpenKey",L"Failed", GetLastError());
		PushErrorMessage();
		return std::wstring();
	}

	if (!(RegQueryValueEx(hKey, L"SteamPath", NULL, &dwType, (LPBYTE)&szPath, &dwPathSize) == ERROR_SUCCESS))
	{

		Log::Message(L"ERROR: %s | %s (%d)\n", L"RegQueryValueEx", L"Failed", GetLastError());
		PushErrorMessage();
		return std::wstring();
	}
	Log::Message(L"INFO: Steam path: %s\n", szPath);


	return std::wstring(szPath,wcslen(szPath));
}

void ProcessLibraryFolders()
{
	SteamCleanup();
	wchar_t buffer[MAX_PATH] = {};
	std::wstring strSteamPath = FindSteamDirectory();

	std::filesystem::current_path(strSteamPath);

	std::wstring strSteamApps = L"steamapps";

	if (std::filesystem::exists(strSteamApps))
	{
		std::filesystem::current_path(strSteamApps);

		Log::Message(L"INFO: Working path: %s\n", std::filesystem::current_path().c_str());

		std::wifstream pLibraryFolders(L"libraryfolders.vdf");

		if (pLibraryFolders)
		{
			auto root = tyti::vdf::read(pLibraryFolders);

			vLibraryPaths.push_back(strSteamPath);

			for (int i = 1; i < root.attribs.size() - 1; i++)
				vLibraryPaths.push_back(root.attribs[std::to_wstring(i)]);
	
			Log::Message(L"INFO: %s | %s \n", L"ProcessLibraryFolders", L"Found game folders:");

			for (int i = 0; i < vLibraryPaths.size(); i++)
				Log::Message(L"INFO: %s \n", vLibraryPaths[i].c_str());
		}
		else
		{
			Log::Message(L"ERROR: %s | %s (%d)\n", L"ProcessLibraryFolders", L"libraryfolders.vdf missing/error: ", GetLastError());
			PushErrorMessage();
		}


	    

	}
	else
	{
		Log::Message(L"ERROR: %s | %s (%d)\n", L"ProcessLibraryFolders", L"Steamapps folder missing/error: ", GetLastError());
		PushErrorMessage();
	}



}

void FindManhuntManifest()
{
	if (vLibraryPaths.size() > 0)
	{
		std::wstring strRequiredACF = L"appmanifest_" + std::to_wstring(MANHUNT_STEAM_ID) + L".acf";

		for (int i = 0; i < vLibraryPaths.size(); i++)
		{

			if (bIsManhuntPathAvailable)
				break;

			std::filesystem::current_path(vLibraryPaths[i]);
			std::filesystem::current_path(L"steamapps");

			std::wstring strCommon = L"common";

			if (std::filesystem::exists(strCommon))
			{
				Log::Message(L"INFO: %s | %s %s\n", L"FindManhuntManifest", L"Scanning: ", std::filesystem::current_path().wstring().c_str());

				for (const auto & file : std::filesystem::directory_iterator(std::filesystem::current_path()))
				{
					if (!std::filesystem::is_directory(file.path().wstring()) && !bIsManhuntPathAvailable)
					{
						if (file.path().extension().wstring() == L".acf")
						{
							std::wstring str = file.path().wstring();
							int fnd = str.find_last_of(L"/\\");

							std::wstring strACF = str.substr(fnd + 1);


							if (wcscmp(strACF.c_str(),strRequiredACF.c_str())  == 0)
							{
								std::wifstream pACF(file.path().wstring());
								auto root = tyti::vdf::read(pACF);
								std::wstring installpath = root.attribs[L"installdir"];
								strManhuntPath = std::filesystem::current_path().wstring() + L"\\" +strCommon + L"\\" + installpath;
								Log::Message(L"INFO: %s | %s %s \n", L"FindManhuntManifest", L"Manhunt location:  ", strManhuntPath.c_str());
								bIsManhuntPathAvailable = true;
								break;
							}

						}
					}
				}

			}
			else
			{
				Log::Message(L"ERROR: %s | %s (%d)\n", L"FindManhuntManifest", L"Common folder missing/error: ", GetLastError());
				PushErrorMessage();
				break;
			}
		}
		if (!bIsManhuntPathAvailable)
		{
			Log::Message(L"ERROR: %s | %s\n", L"FindManhuntManifest", L"Could not find valid Manhunt installation");
			PushErrorMessage();
		}
	}
	else {
		Log::Message(L"ERROR: %s | %s \n", L"FindManhuntManifest", L"No library folders available");
		PushErrorMessage();
	}
}

std::wstring GetFinalSteamManhuntPath()
{
	return strManhuntPath;
}

void SteamCleanup()
{
	vLibraryPaths.clear();
	strManhuntPath = std::wstring();
	bIsManhuntPathAvailable = false;
}
