// exepatch.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <filesystem>
#include <Windows.h>

int main()
{
	WORD dwFlags = 0;
	DWORD dwOffset = 0x8DE;

	if (std::experimental::filesystem::exists(L"testapp.exe"))
	{
		std::experimental::filesystem::rename(L"manhunt.exe", L"manhunt.exe.backup");
		std::experimental::filesystem::rename(L"testapp.exe", L"manhunt.exe");
		MessageBoxA(0,"INFO: A copy of previous executable has been saved as manhunt.exe.backup. You may remove it on your own\n",0,0);
	}
	std::ifstream pReadExecutable("manhunt.exe", std::ifstream::binary);

	if (pReadExecutable)
	{
		pReadExecutable.seekg(dwOffset, std::ifstream::beg);
		pReadExecutable.read((char*)&dwFlags, sizeof(WORD));
		pReadExecutable.close();

		if (!(dwFlags & (IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE | IMAGE_DLLCHARACTERISTICS_NX_COMPAT)))
		{
			MessageBoxA(0,"INFO: Executable is already patched.\n",0,MB_ICONINFORMATION);
		}
		else
		{
			dwFlags &= ~(IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE | IMAGE_DLLCHARACTERISTICS_NX_COMPAT);
			std::ofstream pExecutable(L"manhunt.exe", std::wofstream::binary | std::wofstream::in | std::wofstream::out);
			pExecutable.seekp(dwOffset, std::ofstream::beg);
			pExecutable.write((char*)&dwFlags, sizeof(WORD));
			pExecutable.close();
			MessageBoxA(0, "INFO: Executable patched.\n", 0, MB_ICONINFORMATION);
		}
	}
	else
	{
		MessageBoxA(0,"ERROR: Failed to open manhunt.exe!\n",0,MB_ICONERROR);
		return 0;
	}

	MessageBoxA(0,"INFO: Finished.",0,0);
	return 0;
}

