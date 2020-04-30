#pragma once

#include <credentialprovider.h>
#include "CUnknown.h"
#include "RDPState.h"

// forward references
class CBaseCredential;
class CSampleCredential;
class CMessageCredential;

class CSampleProvider : public CUnknown<ICredentialProvider>
{
public: // ICredentialProvider

    HRESULT SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags) override;
    HRESULT SetSerialization(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION *) NOT_IMPLEMENTED;

    HRESULT Advise(ICredentialProviderEvents* pcpe, UINT_PTR upAdviseContext) override;
    HRESULT UnAdvise() override;

    HRESULT GetFieldDescriptorCount(DWORD* pdwCount) override;
    HRESULT GetFieldDescriptorAt(DWORD dwIndex, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd) override;

    HRESULT GetCredentialCount(DWORD* pdwCount, DWORD* pdwDefault, BOOL* pbAutoLogonWithDefault) override;
    HRESULT GetCredentialAt(DWORD dwIndex, ICredentialProviderCredential** ppcpc) override;

    static const GUID CLSID;

private:

    friend HRESULT CreateInstance<CSampleProvider>(REFIID riid, void** ppv);

    CSampleProvider() = default;
    virtual ~CSampleProvider() { cleanup(); }

    void OnConnectStatusChanged();
    static void OnConnectStatusChanged(CSampleProvider *self) {
        self->OnConnectStatusChanged();
    }

    void cleanup();

    CBaseCredential *getCredential();

private:
    RDPState                    _rdpState;
    CSampleCredential           *_pCredential{}; // Our "connected" credential.
    CMessageCredential          *_pMessageCredential{};   // Our "disconnected" credential.
    ICredentialProviderEvents   *_pcpe{};                    // Used to tell our owner to re-enumerate credentials.
    UINT_PTR                    _upAdviseContext{};       // Used to tell our owner who we are when asking to 
                                                        // re-enumerate credentials.
    CREDENTIAL_PROVIDER_USAGE_SCENARIO      _cpus{};
};
