#pragma once

#include "common.h"
#include "dll.h"
#include <shlwapi.h>

#define NOT_IMPLEMENTED override { return E_NOTIMPL; }

class CBaseCredential : public ICredentialProviderCredential
{
    LONG _cRef;

public:
    CBaseCredential(): _cRef(1) { DllAddRef(); }
    virtual ~CBaseCredential() { DllRelease(); }

public: // IUnknown

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        return ++_cRef;
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        LONG cRef = --_cRef;
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(__in REFIID riid, __deref_out void** ppv) override
    {
        static const QITAB qit[] =
        {
            QITABENT(CBaseCredential, ICredentialProviderCredential), // IID_ICredentialProviderCredential
            {0},
        };
        return QISearch(this, qit, riid, ppv);
    }

public: // ICredentialProviderCredential default implementation for all field-related methods

    IFACEMETHODIMP GetStringValue(DWORD, PWSTR*) NOT_IMPLEMENTED;
    IFACEMETHODIMP GetBitmapValue(DWORD, HBITMAP*) NOT_IMPLEMENTED;
    IFACEMETHODIMP GetCheckboxValue(DWORD, BOOL*, PWSTR*) NOT_IMPLEMENTED;
    IFACEMETHODIMP GetComboBoxValueCount(DWORD, DWORD*, DWORD*) NOT_IMPLEMENTED;
    IFACEMETHODIMP GetComboBoxValueAt(DWORD, DWORD, PWSTR*) NOT_IMPLEMENTED;
    IFACEMETHODIMP GetSubmitButtonValue(DWORD, DWORD*) NOT_IMPLEMENTED;
    IFACEMETHODIMP SetStringValue(DWORD, PCWSTR) NOT_IMPLEMENTED;
    IFACEMETHODIMP SetCheckboxValue(DWORD, BOOL) NOT_IMPLEMENTED;
    IFACEMETHODIMP SetComboBoxSelectedValue(DWORD, DWORD) NOT_IMPLEMENTED;
    IFACEMETHODIMP CommandLinkClicked(DWORD) NOT_IMPLEMENTED;

public: // ICredentialProviderCredential default implementation for other optional methods

    IFACEMETHODIMP Advise(ICredentialProviderCredentialEvents*) NOT_IMPLEMENTED;
    IFACEMETHODIMP UnAdvise() NOT_IMPLEMENTED;

    IFACEMETHODIMP SetSelected(BOOL*) { return S_FALSE; }
    IFACEMETHODIMP SetDeselected() { return S_OK; }

    IFACEMETHODIMP ReportResult(NTSTATUS, NTSTATUS, PWSTR*,
        CREDENTIAL_PROVIDER_STATUS_ICON*) NOT_IMPLEMENTED;
};