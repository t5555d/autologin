//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// CSampleProvider implements ICredentialProvider, which is the main
// interface that logonUI uses to decide which tiles to display.
// This sample illustrates processing asynchronous external events and 
// using them to provide the user with an appropriate set of credentials.
// In this sample, we provide two credentials: one for when the system
// is "connected" and one for when it isn't. When it's "connected", the
// tile provides the user with a field to log in as the administrator.
// Otherwise, the tile asks the user to connect first.
//

#include <credentialprovider.h>
#include "CSampleCredential.h"
#include "CSampleProvider.h"
#include "guid.h"

// CSampleProvider ////////////////////////////////////////////////////////

CSampleProvider::CSampleProvider():
    _cRef(1)
{
    DllAddRef();

    _pcpe = NULL;
    _pCredential = NULL;
    _pMessageCredential = NULL;
}

CSampleProvider::~CSampleProvider()
{
    if (_pCredential != NULL)
    {
        _pCredential->Release();
        _pCredential = NULL;
    }

    DllRelease();
}

// This method acts as a callback for the hardware emulator. When it's called, it simply
// tells the infrastructure that it needs to re-enumerate the credentials.
void CSampleProvider::OnConnectStatusChanged()
{
    if (_rdpState.is_rdp_active()) {
        _pMessageCredential->SetMessage(L"Auto-login is off, due to active RDP connection with %S", _rdpState.get_rdp_text());
    }
    else {
        _pMessageCredential->SetMessage(L"Auto-login is on");
    }

    if (_pcpe != NULL)
    {
        _pcpe->CredentialsChanged(_upAdviseContext);
    }
}

// SetUsageScenario is the provider's cue that it's going to be asked for tiles
// in a subsequent call.
HRESULT CSampleProvider::SetUsageScenario(
    __in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
    __in DWORD dwFlags
)
{
    UNREFERENCED_PARAMETER(dwFlags);

    // Decide which scenarios to support here. Returning E_NOTIMPL simply tells the caller
    // that we're not designed for that scenario.
    switch (cpus)
    {
    case CPUS_LOGON:
    case CPUS_UNLOCK_WORKSTATION:
        break;

    case CPUS_CREDUI:
    case CPUS_CHANGE_PASSWORD:
        return E_NOTIMPL;

    default:
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    _cpus = cpus;

    try {
        if (SUCCEEDED(hr) && !_pCredential)
        {
            _pCredential = new CSampleCredential();
            hr = _pCredential->Initialize(_cpus, s_rgCredProvFieldDescriptors, s_rgFieldStatePairs);
        }

        if (SUCCEEDED(hr) && !_pMessageCredential)
        {
            _pMessageCredential = new CMessageCredential();
            hr = _pMessageCredential->Initialize(s_rgMessageCredProvFieldDescriptors, s_rgMessageFieldStatePairs);
        }

        if (SUCCEEDED(hr))
        {
            _rdpState.set_callback(OnConnectStatusChanged, this);
            _rdpState.start();
        }

    }
    catch (const std::bad_alloc&)
    {
        hr = E_OUTOFMEMORY;
    }

    // If anything failed, clean up.
    if (FAILED(hr))
    {
        if (_pCredential != NULL)
        {
            _pCredential->Release();
            _pCredential = NULL;
        }
        if (_pMessageCredential != NULL)
        {
            _pMessageCredential->Release();
            _pMessageCredential = NULL;
        }
    }

    return hr;
}

// Called by LogonUI to give you a callback. Providers often use the callback if they
// some event would cause them to need to change the set of tiles that they enumerated
HRESULT CSampleProvider::Advise(
    __in ICredentialProviderEvents* pcpe,
    __in UINT_PTR upAdviseContext
    )
{
    if (_pcpe != NULL)
    {
        _pcpe->Release();
    }
    _pcpe = pcpe;
    _pcpe->AddRef();
    _upAdviseContext = upAdviseContext;
    return S_OK;
}

// Called by LogonUI when the ICredentialProviderEvents callback is no longer valid.
HRESULT CSampleProvider::UnAdvise()
{
    if (_pcpe != NULL)
    {
        _pcpe->Release();
        _pcpe = NULL;
    }
    return S_OK;
}

// Called by LogonUI to determine the number of fields in your tiles. We return the number
// of fields to be displayed on our active tile, which depends on our connected state. The
// "connected" CSampleCredential has SFI_NUM_FIELDS fields, whereas the "disconnected" 
// CMessageCredential has SMFI_NUM_FIELDS fields.
HRESULT CSampleProvider::GetFieldDescriptorCount(
    __out DWORD* pdwCount
    )
{
    if (!_rdpState.is_rdp_active())
    {
        *pdwCount = SFI_NUM_FIELDS;
    }
    else
    {
        *pdwCount = SMFI_NUM_FIELDS;
    }
  
    return S_OK;
}

// Gets the field descriptor for a particular field. Note that we need to determine which
// tile to use based on the "connected" status.
HRESULT CSampleProvider::GetFieldDescriptorAt(
    __in DWORD dwIndex, 
    __deref_out CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd
    )
{    
    HRESULT hr;

    if (!_rdpState.is_rdp_active())
    {
        // Verify dwIndex is a valid field.
        if ((dwIndex < SFI_NUM_FIELDS) && ppcpfd)
        {
            hr = FieldDescriptorCoAllocCopy(s_rgCredProvFieldDescriptors[dwIndex], ppcpfd);
        }
        else
        { 
            hr = E_INVALIDARG;
        }
    }
    else
    {
        // Verify dwIndex is a valid field.
        if ((dwIndex < SMFI_NUM_FIELDS) && ppcpfd)
        {
            hr = FieldDescriptorCoAllocCopy(s_rgMessageCredProvFieldDescriptors[dwIndex], ppcpfd);
        }
        else
        { 
            hr = E_INVALIDARG;
        }
    }

    return hr;
}

// We only use one tile at any given time since the system can either be "connected" or 
// "disconnected". If we decided that there were multiple valid ways to be connected with
// different sets of credentials, we would provide a combobox in the "connected" tile so
// that the user could pick which one they want to use.
// The last cred prov used gets to select the default user tile
HRESULT CSampleProvider::GetCredentialCount(
    __out DWORD* pdwCount,
    __out_range(<,*pdwCount) DWORD* pdwDefault,
    __out BOOL* pbAutoLogonWithDefault
    )
{
    *pdwCount = 1;
    *pdwDefault = 0;
    *pbAutoLogonWithDefault = FALSE;
    return S_OK;
}

// Returns the credential at the index specified by dwIndex. This function is called
// to enumerate the tiles. Note that we need to return the right credential, which depends
// on whether we're connected or not.
HRESULT CSampleProvider::GetCredentialAt(
    __in DWORD dwIndex, 
    __deref_out ICredentialProviderCredential** ppcpc
    )
{
    HRESULT hr;
    // Make sure the parameters are valid.
    if ((dwIndex == 0) && ppcpc)
    {
        if (!_rdpState.is_rdp_active())
        {
            hr = _pCredential->QueryInterface(IID_ICredentialProviderCredential, reinterpret_cast<void**>(ppcpc));
        }
        else
        {
            hr = _pMessageCredential->QueryInterface(IID_ICredentialProviderCredential, reinterpret_cast<void**>(ppcpc));
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }
        
    return hr;
}

// Boilerplate method to create an instance of our provider. 
HRESULT CSample_CreateInstance(__in REFIID riid, __in void** ppv)
{
    HRESULT hr;

    CSampleProvider* pProvider = new CSampleProvider();

    if (pProvider)
    {
        hr = pProvider->QueryInterface(riid, ppv);
        pProvider->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}
