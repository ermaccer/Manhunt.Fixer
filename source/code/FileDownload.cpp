#include "FileDownload.h"
#include "TaskManager.h"
#include "Log.h"
#ifdef USE_LIBCURL
#include "..\curl\curl.h"
#endif

std::wstring strFileToDownload;

#ifndef  USE_LIBCURL
STDMETHODIMP_(HRESULT __stdcall) DLCallback::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR wszStatusText)
{
	static wchar_t szProgressReport[512] = {};

	wsprintf((LPWSTR)&szProgressReport, L"Downloading: %s %d KB/%d KB", strFileToDownload.c_str(), ulProgress / 1024, ulProgressMax / 1024);
	TaskManager::TaskSetName(szProgressReport);
	return S_OK;

}

STDMETHODIMP_(HRESULT __stdcall) DLCallback::OnStartBinding(DWORD dwReserved, IBinding * pib)
{
	return E_NOTIMPL;
}

STDMETHODIMP_(HRESULT __stdcall) DLCallback::GetPriority(LONG * pnPriority)
{
	return E_NOTIMPL;
}

STDMETHODIMP_(HRESULT __stdcall) DLCallback::OnLowResource(DWORD reserved)
{
	return E_NOTIMPL;
}

STDMETHODIMP_(HRESULT __stdcall) DLCallback::OnStopBinding(HRESULT hresult, LPCWSTR szError)
{
	return E_NOTIMPL;
}

STDMETHODIMP_(HRESULT __stdcall) DLCallback::GetBindInfo(DWORD * grfBINDF, BINDINFO * pbindinfo)
{
	return E_NOTIMPL;
}

STDMETHODIMP_(HRESULT __stdcall) DLCallback::OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC * pformatetc, STGMEDIUM * pstgmed)
{
	return E_NOTIMPL;
}

STDMETHODIMP_(HRESULT __stdcall) DLCallback::OnObjectAvailable(REFIID riid, IUnknown * punk)
{
	return E_NOTIMPL;
}

STDMETHODIMP_(ULONG __stdcall) DLCallback::AddRef()
{
	return 0;
}

STDMETHODIMP_(ULONG __stdcall) DLCallback::Release()
{
	return 0;
}

STDMETHODIMP_(HRESULT __stdcall) DLCallback::QueryInterface(REFIID riid, void ** ppvObject)
{
	return E_NOTIMPL;
}
#endif

bool RequestDownloadFile(std::wstring url, std::wstring saveAs)
{
	strFileToDownload = saveAs;
#ifndef USE_LIBCURL
	DLCallback callback;
	Log::Message(L"INFO: %s | %s %s\n", L"RequestDownloadFile", L"Attempting to download:", url.c_str());
	if (URLDownloadToFile(NULL, url.c_str(), saveAs.c_str(), 0, &callback) == S_OK)
	{
		Log::Message(L"INFO: %s | %s %s\n", __FUNCTIONW__, L"File downloaded:", saveAs.c_str());
		return true;
	}
	else
	{
		Log::Message(L"ERROR: %s | %s %s\n", __FUNCTIONW__, L"Failed to download:", saveAs.c_str());
		return false;
	}
#else
	CURL* curl;
	CURLcode code;

	curl = curl_easy_init();
	if (!curl)
	{
		Log::Message(L"ERROR: %s | %s\n", __FUNCTIONW__, L"Failed to init libcurl!");
		return false;
	}

	if (curl)
	{
		Log::Message(L"INFO: %s | %s %s\n", __FUNCTIONW__, L"Attempting to download:", url.c_str());
		std::string str("", url.length());
		std::copy(url.begin(), url.end(), str.begin());
		FILE* pFile = _wfopen(saveAs.c_str(), L"wb");
		
		if (pFile)
		{ /* send all data to this function  */
			curl_easy_setopt(curl, CURLOPT_URL, str.c_str());
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, libcurl_download_progress);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, pFile);
			code = curl_easy_perform(curl);
			curl_easy_cleanup(curl);
			fclose(pFile);
			if (code == CURLE_OK)
			{
				Log::Message(L"INFO: %s | %s %s\n", __FUNCTIONW__, L"Downloaded:", saveAs.c_str());
				return true;
			}
			else
			{
				Log::Message(L"ERROR: %s | %s %s %d\n", __FUNCTIONW__, L"Failed to download:", saveAs.c_str(), code);
				return false;
			}
		}
		else
		{
			Log::Message(L"ERROR: %s | %s %s\n", __FUNCTIONW__, L"Failed to open file!",saveAs.c_str());
			return false;
		}


	}

#endif // !USE_LIBCURL

	return true;


}
#ifdef USE_LIBCURL
int libcurl_download_progress(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	if (dltotal <= 0.0f)
		return 0;

	static wchar_t szProgressReport[512] = {};
	float prog = dlnow / dltotal;
	prog *= 100.0f;

	wsprintf((LPWSTR)&szProgressReport, L"Downloading: %s %d%%", strFileToDownload.c_str(),(int)prog);
	TaskManager::TaskSetName(szProgressReport);

	return 0;
}
#endif