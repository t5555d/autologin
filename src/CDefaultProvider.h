#pragma once

#include <credentialprovider.h>
#include "CUnknown.h"
#include "RDPState.h"

// forward references
class CBaseCredential;
class CDefaultCredential;
class CMessageCredential;

class CDefaultProvider : public CUnknown<ICredentialProvider>
{
public: // ICredentialProvider

    HRESULT STDCALL SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags) override;
    HRESULT STDCALL SetSerialization(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION *) NOT_IMPLEMENTED;
    HRESULT STDCALL Advise(ICredentialProviderEvents* pcpe, UINT_PTR upAdviseContext) override;
    HRESULT STDCALL UnAdvise() override;
    HRESULT STDCALL GetFieldDescriptorCount(DWORD* pdwCount) override;
    HRESULT STDCALL GetFieldDescriptorAt(DWORD dwIndex, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd) override;
    HRESULT STDCALL GetCredentialCount(DWORD* pdwCount, DWORD* pdwDefault, BOOL* pbAutoLogonWithDefault) override;
    HRESULT STDCALL GetCredentialAt(DWORD dwIndex, ICredentialProviderCredential** ppcpc) override;

    static const GUID CLSID;

private:

    friend HRESULT CreateInstance<CDefaultProvider>(REFIID riid, void** ppv);

    CDefaultProvider() = default;
    virtual ~CDefaultProvider() { cleanup(); }

    void OnConnectStatusChanged();
    static void OnConnectStatusChanged(CDefaultProvider *self) {
        self->OnConnectStatusChanged();
    }

    void cleanup();

    CBaseCredential *getCredential();

private:
    RDPState                    rdpState;
    CDefaultCredential *        defaultCredential{}; // Our "connected" credential.
    CMessageCredential *        messageCredential{}; // Our "disconnected" credential.
    ICredentialProviderEvents * events{};
    UINT_PTR                    eventsContext{};
    CREDENTIAL_PROVIDER_USAGE_SCENARIO usage{};
};
