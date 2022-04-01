#include "GameData.h"
#include "Log.h"
#include "Error.h"
#include "Common.h"
#include <string>
#include <filesystem>

std::wstring strBadFilesToCheck[] = {
	L"AUDIO\\PC\\SCRIPTED\\CRAPPER\\CR@PPER.RIB",
	L"AUDIO\\PC\\SCRIPTED\\GASPOUR\\G@SPOUR.RIB",
	L"AUDIO\\PC\\SCRIPTED\\PORN\\P0RN.RIB",
	L"MANHUNT FIX.BAT",
};



std::wstring strFilesToCheck[] = {
	L"AUDIO\\PC\\SCRIPTED\\TRAIN\\TRAIN1.RIB",
	L"AUDIO\\PC\\SCRIPTED\\TRAIN\\TRAIN2.RIB",
	L"AUDIO\\PC\\SCRIPTED\\TRAIN\\TRAIN3.RIB",
	L"AUDIO\\PC\\SCRIPTED\\PIGGRILL\\PIGGRIL1.RIB",
	L"AUDIO\\PC\\SCRIPTED\\PIGGRILL\\PIGGRIL2.RIB",
	L"AUDIO\\PC\\SCRIPTED\\CRAPPER\\CRAPPER.RIB",
	L"AUDIO\\PC\\SCRIPTED\\GASPOUR\\GASPOUR.RIB",
	L"AUDIO\\PC\\SCRIPTED\\JOURNO\\JOURNO.RIB",
	L"AUDIO\\PC\\SCRIPTED\\PORN\\PORN.RIB",
	L"AUDIO\\PC\\SCRIPTED\\DIRECT\\DIRECT.RIB",
	L"AUDIO\\PC\\SCRIPTED\\DDEATH\\DDEATH.RIB",
	L"AUDIO\\PC\\SCRIPTED\\DDOOR\\DDOOR.RIB",
	L"AUDIO\\PC\\SCRIPTED\\PDOOR\\PDOOR.RIB",
	L"AUDIO\\PC\\SCRIPTED\\MALLTV\\MALLTV.RIB",
};



bool VerifyManhuntGameData()
{
	for (int i = 0; i < 4; i++)
	{
		if (std::filesystem::exists(strBadFilesToCheck[i]))
		{
			if (MessageBox(gHWND, L"One or more changes featured in other fix have been found in your Manhunt directory. "
				"Please remove added files them or rename files back to original filename. Revert your NVIDIA Control Panel settings, if you have made any changes."
				" \n\n"
				"Use this tool on a clean game installation.\n"
				"Press ""OK"" to dismiss this warning", TOOL_NAME, MB_ICONSTOP) == IDOK)
			{
				break;
			}

		}

	}


	for (int i = 0; i < 14; i++)
	{
		if (!std::filesystem::exists(strFilesToCheck[i]))
		{
			Log::Message(L"ERROR: %s | %s %s\n", L"VerifyManhuntGameData", L"Could not find file:", strFilesToCheck[i].c_str());
			PushErrorMessage();
			return false;
			break;
		}
		else
			Log::Message(L"INFO: %s | %s %s\n", L"VerifyManhuntGameData", L"File found:", strFilesToCheck[i].c_str());

	}


	return true;
}
