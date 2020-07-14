#pragma once
#include <Windows.h>
#include <string>



namespace TaskManager {
	void InitItemsToDownload();
	void ClearDownloads();
	void AddDownload(std::wstring url, std::wstring file);
	void LinkItems(HWND* bar, HWND* tname, std::wstring gamePath);
	void TaskSetName(std::wstring name);
	void TaskBegin(std::wstring name);
	bool TaskEnd(bool* val);
	bool TaskVerifyGameData();
	bool TaskDownloadRequiredFiles();
	bool TaskUnzipDownloadedFiles();
	bool TaskPossiblyDeleteExistingFiles();
	bool TaskPatchExecutable();
	bool TaskApply60FPSLimit();
	void TaskComplete();
}

void AskForPMHSettings();