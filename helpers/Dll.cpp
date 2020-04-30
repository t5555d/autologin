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
#include "Dll.h"
#include "helpers.h"

static LONG g_cRef = 0;   // global dll reference count

extern HRESULT CSample_CreateInstance(REFIID riid, void** ppv);
EXTERN_C GUID CLSID_CSample;

class CClassFactory : public IClassFactory
{
public:
    CClassFactory() : _cRef(1) 
    {
    }

    // IUnknown
    HRESULT QueryInterface(REFIID riid, void **ppv) override
    {
        static const QITAB qit[] = 
        {
            QITABENT(CClassFactory, IClassFactory),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    ULONG AddRef() override
    {
        return InterlockedIncrement(&_cRef);
    }

    ULONG Release() override
    {
        LONG cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
            delete this;
        return cRef;
    }

    // IClassFactory
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

private:
    ~CClassFactory()
    {
    }
    long _cRef;
};

HRESULT CClassFactory_CreateInstance(REFCLSID rclsid, REFIID riid, void **ppv)
{
    *ppv = NULL;

    HRESULT hr;

    if (CLSID_CSample == rclsid)
    {
        CClassFactory* pcf = new CClassFactory();
        if (pcf)
        {
            hr = pcf->QueryInterface(riid, ppv);
            pcf->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = CLASS_E_CLASSNOTAVAILABLE;
    }
    return hr;
}

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

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    return CClassFactory_CreateInstance(rclsid, riid, ppv);
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