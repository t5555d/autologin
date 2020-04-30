//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Standard dll required functions and class factory implementation.

#include <windows.h>
#include <unknwn.h>
#include <WinCred.h>
#include "CUnknown.h"
#include "CSampleProvider.h"

static LONG g_cRef = 0;   // global dll reference count

void DllAddRef()
{
    InterlockedIncrement(&g_cRef);
}

void DllRelease()
{
    InterlockedDecrement(&g_cRef);
}

STDAPI DllCanUnloadNow()
{
    return (g_cRef > 0) ? S_FALSE : S_OK;
}

extern HRESULT CSample_CreateInstance(REFIID riid, void** ppv);

class CClassFactory : public CUnknown<IClassFactory>
{
public: // IClassFactory

    HRESULT CreateInstance(IUnknown* pUnkOuter, REFIID riid, void **ppv) override
    {
        HRESULT hr;
        if (!pUnkOuter)
        {
            hr = CSample_CreateInstance(riid, ppv);
        }
        else
        {
            *ppv = NULL;
            hr = CLASS_E_NOAGGREGATION;
        }
        return hr;
    }

    HRESULT LockServer(BOOL bLock) override
    {
        if (bLock)
        {
            DllAddRef();
        }
        else
        {
            DllRelease();
        }
        return S_OK;
    }
};

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    *ppv = NULL;

    if (rclsid == CSampleProvider::CLSID)
        return CreateInstance<CClassFactory>(riid, ppv);

    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI_(BOOL) DllMain(HINSTANCE hinstDll, DWORD dwReason, void *)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDll);
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    
    return TRUE;
}

STDAPI DllTest()
{
    BOOL save = false;
    DWORD authPackage = 0;
    LPVOID authBuffer;
    ULONG authBufferSize = 0;
    CREDUI_INFO credUiInfo;

    credUiInfo.pszCaptionText = TEXT("Test Credential Providers");
    credUiInfo.pszMessageText = TEXT("Test Credential Providers");
    credUiInfo.cbSize = sizeof(credUiInfo);
    credUiInfo.hbmBanner = NULL;
    credUiInfo.hwndParent = NULL;

    DWORD error = CredUIPromptForWindowsCredentials(&credUiInfo, 0, &authPackage, NULL, 0, &authBuffer, &authBufferSize, &save, 0);
    return HRESULT_FROM_WIN32(error);
}