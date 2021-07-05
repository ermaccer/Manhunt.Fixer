// Fixer.cpp : Defines the entry point for the application.
//


#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib,"zipper.lib")
#pragma comment(lib,"zlibstat.lib")
#include "framework.h"
#include "Fixer.h"
#include <string>
#include <CommCtrl.h>

#include "code/Common.h"
#include "code/Log.h"
#include "code/SteamDirectory.h"
#include "code/Error.h"
#include "code/GameData.h"
#include "code/TaskManager.h"


HINSTANCE gInstance;


INT_PTR CALLBACK    DlgProc(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK    AboutBox(HWND, UINT, WPARAM, LPARAM);



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	gInstance = hInstance;
	DialogBox(gInstance, MAKEINTRESOURCE(IDD_DIALOG1), 0, DlgProc);


}


HWND hInputPath = 0;
HWND hBar = 0;
HWND hCurrentTask = 0;
HWND GlobalHWND = 0;

void CreateTooltip(HWND hParent, LPCWSTR Text)
{
	HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP |
		TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hParent, NULL,
		gInstance, NULL);
	SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	TOOLINFO ti;
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
	ti.hwnd = GlobalHWND;
	ti.hinst = NULL;
	ti.uId = (UINT_PTR)hParent;
	ti.lpszText = (LPWSTR)Text;

	RECT rect;
	GetClientRect(hParent, &rect);

	ti.rect.left = rect.left;
	ti.rect.top = rect.top;
	ti.rect.right = rect.right;
	ti.rect.bottom = rect.bottom;

	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)&ti);
} 



INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	GlobalHWND = hDlg;
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
	
		Log::Clear();
		Log::Init();
		hInputPath = GetDlgItem(hDlg, MANHUNT_PATH);
		hBar = GetDlgItem(hDlg, PROGBAR);
		hCurrentTask = GetDlgItem(hDlg, TASK_INFO);
		TaskManager::InitItemsToDownload();

		CreateTooltip(GetDlgItem(hDlg, ENH_WSTEXT), L"Replaces frontend textures with rescaled 1920x1080 ones.");
		CreateTooltip(GetDlgItem(hDlg, ENH_PMH), L"Allows to skip Legal screen, access debug menu and more.");
		CreateTooltip(GetDlgItem(hDlg, ENH_BLDFIX), L"Replaces wrong player blood/damage textures with fixed ones so the effect looks like on PS2. (File - frontend_pc.txd)");
		CreateTooltip(GetDlgItem(hDlg, ENH_PS2CASH), L"PS2 texture for player model. (File - cash_pc.txd)");
		CreateTooltip(GetDlgItem(hDlg, ENH_MDLFIX), L"Fixes missing texture on rats & crows (File - gmodelspc.dff)");
		CreateTooltip(GetDlgItem(hDlg, ENH_PS2HALOS), L"Improves game renderer by making it work like PS2 version, also includes PS2 halos");
		CreateTooltip(GetDlgItem(hDlg, ENH_DISCORD), L"Shows your current level, kills & executions in Discord.");
        return (INT_PTR)TRUE;

	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		return (INT_PTR)TRUE;



    case WM_COMMAND:
		if (wParam == LOCATE_STEAM)
		{
			ProcessLibraryFolders();
			FindManhuntManifest();
			std::wstring strManhuntPath = GetFinalSteamManhuntPath();
			if (strManhuntPath.length() > 0)
			{
				SetWindowText(hInputPath, strManhuntPath.c_str());
			}
		}
		if (wParam == SELECT_MANHUNT_PATH)
			SetWindowText(hInputPath, SetFolderFromButton(hDlg).c_str());
		if (wParam == ID_FILE_LOG)
			Log::ShowLog();
		if (wParam == IDM_EXIT)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		if (wParam == IDM_ABOUT)
		{
			DialogBox(gInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, AboutBox);
		}
		if (wParam == PATCH)
		{
			TaskManager::ClearDownloads();
			hInputPath = GetDlgItem(hDlg, MANHUNT_PATH);
			if (GetWindowTextLength(hInputPath) == 0)
			{
				MessageBox(GlobalHWND, L"Specify Manhunt path!", TOOL_NAME, MB_ICONWARNING);
				break;
			}
			wchar_t szInPath[MAX_PATH] = {};
			GetWindowText(hInputPath, szInPath, GetWindowTextLength(hInputPath) + 1);
			std::wstring tmp = szInPath;

			if (ValidatePath(tmp))
			{
				TaskManager::LinkItems(&hBar, &hCurrentTask, tmp);
				if (TaskManager::TaskVerifyGameData())
				{
					if (IsDlgButtonChecked(hDlg, ENH_WSTEXT))
						TaskManager::AddDownload(L"https://github.com/ThirteenAG/WidescreenFixesPack/releases/download/manhunt/Manhunt.WidescreenFrontend.zip", L"Manhunt.WidescreenFrontend.zip");

					if (IsDlgButtonChecked(hDlg, ENH_BLDFIX))
						TaskManager::AddDownload(L"https://github.com/ermaccer/Manhunt.Fixer/raw/master/bloodfix.zip", L"bloodfix.zip");

					if (IsDlgButtonChecked(hDlg, ENH_PS2CASH))
						TaskManager::AddDownload(L"https://github.com/ermaccer/Manhunt.Fixer/raw/master/ps2cash.zip", L"ps2cash.zip");

					if (IsDlgButtonChecked(hDlg, ENH_MDLFIX))
						TaskManager::AddDownload(L"https://github.com/ermaccer/Manhunt.Fixer/raw/master/gmodels_fix.zip", L"gmodels_fix.zip");

					if (IsDlgButtonChecked(hDlg, ENH_PMH))
						TaskManager::AddDownload(L"https://github.com/ermaccer/Manhunt.PluginMH/releases/latest/download/PluginMH.zip", L"PluginMH.zip");

					if (IsDlgButtonChecked(hDlg, ENH_PS2HALOS))
						TaskManager::AddDownload(L"https://github.com/Fire-Head/MHP/releases/latest/download/MHP.zip", L"MHP.zip");

					if (IsDlgButtonChecked(hDlg, ENH_DISCORD))
						TaskManager::AddDownload(L"https://github.com/ermaccer/Manhunt.DiscordPlugin/releases/latest/download/DiscordPlugin.zip", L"DiscordPlugin.zip");

					if (TaskManager::TaskDownloadRequiredFiles())
					if (TaskManager::TaskPossiblyDeleteExistingFiles())
					if (TaskManager::TaskUnzipDownloadedFiles())
					if (TaskManager::TaskPatchExecutable())
					if (TaskManager::TaskApply60FPSLimit())
					{
						TaskManager::TaskComplete();
						TaskManager::ClearDownloads();
						PushErrorMessage(ERROR_COMPLETE);
						if (IsDlgButtonChecked(hDlg, ENH_PMH))
							AskForPMHSettings();
					}
								
				}
			}
		}


        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void PushLogMessage(HWND hWnd, std::wstring msg)
{
	msg += L"\r\n";
	int len = SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0);
	SendMessage(hWnd, EM_SETSEL, (WPARAM)len, (LPARAM)len);
	SendMessage(hWnd, EM_REPLACESEL, FALSE, (LPARAM)msg.c_str());

}


INT_PTR CALLBACK AboutBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		PushLogMessage(GetDlgItem(hDlg, CREDITS_BOX), L"ermaccer - tool programming, executable patch, audio fix, model fixes");
		PushLogMessage(GetDlgItem(hDlg, CREDITS_BOX), L"ThirteenAG - widescreen fix");
		PushLogMessage(GetDlgItem(hDlg, CREDITS_BOX), L"Fire_Head - mhpatches, model fixes, code support");
		PushLogMessage(GetDlgItem(hDlg, CREDITS_BOX), L"Ash_735 - graphics design");
		PushLogMessage(GetDlgItem(hDlg, CREDITS_BOX), L"Testers:");
		PushLogMessage(GetDlgItem(hDlg, CREDITS_BOX), L"ZT, Dominik, Barbo, 5d0, Geisteskrankenchan, Zach Fett");
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}