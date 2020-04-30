//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#pragma once

#include <credentialprovider.h>
#include <windows.h>
#include <strsafe.h>

#include "CSampleCredential.h"
#include "MessageCredential.h"
#include "helpers.h"
#include "RDPState.h"

// Forward references for classes used here.
class CSampleCredential;
class CMessageCredential;

class CSampleProvider : public ICredentialProvider
{
  public:
    // IUnknown
    ULONG AddRef()
    {
        return ++_cRef;
    }
    
    ULONG Release()
    {
        LONG cRef = --_cRef;
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    HRESULT QueryInterface(REFIID riid, void** ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CSampleProvider, ICredentialProvider), // IID_ICredentialProvider
            {0},
        };
        return QISearch(this, qit, riid, ppv);
    }

  public:
    HRESULT SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags);
    HRESULT SetSerialization(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION *) { return E_NOTIMPL; }

    HRESULT Advise(ICredentialProviderEvents* pcpe, UINT_PTR upAdviseContext);
    HRESULT UnAdvise();

    HRESULT GetFieldDescriptorCount(DWORD* pdwCount);
    HRESULT GetFieldDescriptorAt(DWORD dwIndex, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd);

    HRESULT GetCredentialCount(DWORD* pdwCount, DWORD* pdwDefault, BOOL* pbAutoLogonWithDefault);
    HRESULT GetCredentialAt(DWORD dwIndex, ICredentialProviderCredential** ppcpc);

    friend HRESULT CSample_CreateInstance(REFIID riid, void** ppv);

    void OnConnectStatusChanged();
private:

    CSampleProvider();
    virtual ~CSampleProvider();

    static void OnConnectStatusChanged(CSampleProvider *self) {
        self->OnConnectStatusChanged();
    }

    void cleanup();

    CBaseCredential *getCredential();

private:
    LONG                        _cRef;                  // Reference counter.
    RDPState                    _rdpState;
    CSampleCredential           *_pCredential;          // Our "connected" credential.
    CMessageCredential          *_pMessageCredential;   // Our "disconnected" credential.
    ICredentialProviderEvents   *_pcpe;                    // Used to tell our owner to re-enumerate credentials.
    UINT_PTR                    _upAdviseContext;       // Used to tell our owner who we are when asking to 
                                                        // re-enumerate credentials.
    CREDENTIAL_PROVIDER_USAGE_SCENARIO      _cpus;
};
