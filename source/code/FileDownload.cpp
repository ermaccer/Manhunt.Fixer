#include "FileDownload.h"
#include "TaskManager.h"
#include "Log.h"

std::wstring strFileToDownload;


STDMETHODIMP_(HRESULT __stdcall) DLCallback::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR wszStatusText)
{
	wchar_t szProgressReport[512] = {};

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

bool RequestDownloadFile(std::wstring url, std::wstring saveAs)
{


	strFileToDownload = saveAs;
	DLCallback callback;
	Log::Message(L"INFO: %s | %s %s\n", L"RequestDownloadFile", L"Attempting to download:", url.c_str());
	if (URLDownloadToFile(NULL, url.c_str(), saveAs.c_str(), 0, &callback) == S_OK)
	{
		Log::Message(L"INFO: %s | %s %s\n", L"RequestDownloadFile", L"File downloaded:", saveAs.c_str());
		return true;
	}
	else
	{
		Log::Message(L"ERROR: %s | %s %s\n", L"RequestDownloadFile", L"Failed to download:", saveAs.c_str());
		return false;
	}


}
