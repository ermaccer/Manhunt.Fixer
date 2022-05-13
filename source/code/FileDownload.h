#pragma once
#include <iostream>
#include <conio.h>
#include <urlmon.h>
#include <string>
#include <cstdlib>

#ifndef  USE_LIBCURL
class DLCallback
	: public IBindStatusCallback
{
public:
	DLCallback() { }
	~DLCallback() { }

	STDMETHOD(OnProgress)(ULONG ulProgress,ULONG ulProgressMax, ULONG ulStatusCode,LPCWSTR wszStatusText);


	STDMETHOD(OnStartBinding)( /* [in] */ DWORD dwReserved, /* [in] */ IBinding __RPC_FAR * pib);
	STDMETHOD(GetPriority)( /* [out] */ LONG __RPC_FAR * pnPriority);
	STDMETHOD(OnLowResource)( /* [in] */ DWORD reserved);
	STDMETHOD(OnStopBinding)( /* [in] */ HRESULT hresult, /* [unique][in] */ LPCWSTR szError);
	STDMETHOD(GetBindInfo)( /* [out] */ DWORD __RPC_FAR * grfBINDF, /* [unique][out][in] */ BINDINFO __RPC_FAR * pbindinfo);
	STDMETHOD(OnDataAvailable)( /* [in] */ DWORD grfBSCF, /* [in] */ DWORD dwSize, /* [in] */ FORMATETC __RPC_FAR * pformatetc, /* [in] */ STGMEDIUM __RPC_FAR * pstgmed);
	STDMETHOD(OnObjectAvailable)( /* [in] */ REFIID riid, /* [iid_is][in] */ IUnknown __RPC_FAR * punk);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD(QueryInterface)( /* [in] */ REFIID riid, /* [iid_is][out] */ void __RPC_FAR * __RPC_FAR * ppvObject);
};
#endif
bool RequestDownloadFile(std::wstring url, std::wstring saveA);

#ifdef  USE_LIBCURL
int libcurl_download_progress(void* clientp, double dltotal,double dlnow,double ultotal,double ulnow);
#endif //  USE_LIBCURL
