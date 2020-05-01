#include <windows.h>
#include <WinCred.h>
#include "CUnknown.h"
#include "CDefaultProvider.h"

HINSTANCE g_instance;
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

class CClassFactory : public CUnknown<IClassFactory>
{
public: // IClassFactory

    HRESULT STDCALL CreateInstance(IUnknown* pUnkOuter, REFIID riid, void **ppv) override
    {
        if (!pUnkOuter)
        {
            return ::CreateInstance<CDefaultProvider>(riid, ppv);
        }
        else
        {
            *ppv = NULL;
            return CLASS_E_NOAGGREGATION;
        }
    }

    HRESULT STDCALL LockServer(BOOL bLock) override
    {
        bLock ? DllAddRef() : DllRelease();
        return S_OK;
    }
};

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    *ppv = NULL;

    if (rclsid == CDefaultProvider::CLSID)
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
    
    g_instance = hinstDll;
    return TRUE;
}

STDAPI DllTest()
{
    BOOL save = false;
    DWORD authPackage = 0;
    LPVOID authBuffer;
    ULONG authBufferSize = 0;
    CREDUI_INFOW credUiInfo;

    credUiInfo.pszCaptionText = L"Test Credential Providers";
    credUiInfo.pszMessageText = L"Test Credential Providers";
    credUiInfo.cbSize = sizeof(credUiInfo);
    credUiInfo.hbmBanner = NULL;
    credUiInfo.hwndParent = NULL;

    // I'm not sure why but it seems that CredUIPromptForWindowsCredentialsA 
    // always return ERROR_GEN_FAILURE (0x1E). Only Unicode version works.
    DWORD error = CredUIPromptForWindowsCredentialsW(&credUiInfo, 0, &authPackage, NULL, 0, &authBuffer, &authBufferSize, &save, 0);
    return HRESULT_FROM_WIN32(error);
}