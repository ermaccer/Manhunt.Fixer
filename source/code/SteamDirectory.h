#pragma once
#include <string>

std::wstring FindSteamDirectory();
void         ProcessLibraryFolders();
void         FindManhuntManifest();
std::wstring GetFinalSteamManhuntPath();

void         SteamCleanup();