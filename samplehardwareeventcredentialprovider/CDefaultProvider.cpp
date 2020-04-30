#include <credentialprovider.h>
#include "CDefaultProvider.h"
#include "CDefaultCredential.h"
#include "CMessageCredential.h"

const GUID CDefaultProvider::CLSID =
{ 0x75a22df0, 0xb81d, 0x46ed, { 0xb1, 0x19, 0xcd, 0x30, 0x50, 0x7b, 0xd6, 0x15 } };

void CDefaultProvider::cleanup()
{
    if (defaultCredential)
    {
        defaultCredential->Release();
        defaultCredential = NULL;
    }
    if (messageCredential)
    {
        messageCredential->Release();
        messageCredential = NULL;
    }
}

// This method acts as a callback for the hardware emulator. When it's called, it simply
// tells the infrastructure that it needs to re-enumerate the credentials.
void CDefaultProvider::OnConnectStatusChanged()
{
    if (rdpState.isActive()) {
        messageCredential->FormatStringValue(MSG_MESSAGE, L"Auto-login is off, due to active RDP connection with %S", rdpState.getPeerText());
    }
    else {
        messageCredential->SetStringValue(MSG_MESSAGE, L"Auto-login is on");
    }

    if (events != NULL)
    {
        events->CredentialsChanged(eventsContext);
    }
}

CBaseCredential *CDefaultProvider::getCredential()
{
    if (rdpState.isActive())
        return messageCredential;
    else
        return defaultCredential;
}

// SetUsageScenario is the provider's cue that it's going to be asked for tiles
// in a subsequent call.
HRESULT CDefaultProvider::SetUsageScenario(
    CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
    DWORD dwFlags
)
{
    UNREFERENCED_PARAMETER(dwFlags);

    // Decide which scenarios to support here. Returning E_NOTIMPL simply tells the caller
    // that we're not designed for that scenario.
    switch (cpus)
    {
    case CPUS_LOGON:
    case CPUS_UNLOCK_WORKSTATION:
    case CPUS_CREDUI:
        break;

    case CPUS_CHANGE_PASSWORD:
        return E_NOTIMPL;

    default:
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    usage = cpus;

    try {
        auto title = L"Auto-login";
        auto username = L"testbot";
        auto password = L"Test1234";

        if (SUCCEEDED(hr) && !defaultCredential)
            defaultCredential = new CDefaultCredential();

        if (SUCCEEDED(hr))
            hr = defaultCredential->SetUsage(usage);

        if (SUCCEEDED(hr))
            hr = defaultCredential->SetStringValue(AUTO_TITLE, title);

        if (SUCCEEDED(hr))
            hr = defaultCredential->SetStringValue(AUTO_USERNAME, username);

        if (SUCCEEDED(hr))
            hr = defaultCredential->SetStringValue(AUTO_PASSWORD, password);

        if (SUCCEEDED(hr) && !messageCredential)
            messageCredential = new CMessageCredential();

        if (SUCCEEDED(hr))
            hr = messageCredential->SetStringValue(MSG_TITLE, title);

        if (SUCCEEDED(hr))
        {
            rdpState.setCallback(OnConnectStatusChanged, this);
            rdpState.start();
        }
    }
    catch (const std::bad_alloc&)
    {
        hr = E_OUTOFMEMORY;
    }

    if (FAILED(hr))
        cleanup();

    return hr;
}

// Called by LogonUI to give you a callback. Providers often use the callback if they
// some event would cause them to need to change the set of tiles that they enumerated
HRESULT CDefaultProvider::Advise(
    ICredentialProviderEvents* pcpe,
    UINT_PTR upAdviseContext
    )
{
    UnAdvise();
    events = pcpe;
    events->AddRef();
    eventsContext = upAdviseContext;
    return S_OK;
}

// Called by LogonUI when the ICredentialProviderEvents callback is no longer valid.
HRESULT CDefaultProvider::UnAdvise()
{
    if (events != NULL)
    {
        events->Release();
        events = NULL;
    }
    return S_OK;
}

// Called by LogonUI to determine the number of fields in your tiles. We return the number
// of fields to be displayed on our active tile, which depends on our "connected" state.
HRESULT CDefaultProvider::GetFieldDescriptorCount(DWORD* pdwCount)
{
    auto cred = getCredential();
    return cred->GetFieldDescriptorCount(pdwCount);
}

// Gets the field descriptor for a particular field. Note that we need to determine which
// tile to use based on the "connected" status.
HRESULT CDefaultProvider::GetFieldDescriptorAt(DWORD dwIndex, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** desc)
{
    auto cred = getCredential();
    return cred->GetFieldDescriptorAt(dwIndex, desc);
}

// We only use one tile at any given time since the system can either be "connected" or 
// "disconnected". If we decided that there were multiple valid ways to be connected with
// different sets of credentials, we would provide a combobox in the "connected" tile so
// that the user could pick which one they want to use.
// The last cred prov used gets to select the default user tile
HRESULT CDefaultProvider::GetCredentialCount(DWORD* pdwCount, DWORD* pdwDefault, BOOL* pbAutoLogonWithDefault)
{
    *pdwCount = 1;
    *pdwDefault = 0;
    *pbAutoLogonWithDefault = FALSE;
    return S_OK;
}

// Returns the credential at the index specified by dwIndex. This function is called
// to enumerate the tiles. Note that we need to return the right credential, which depends
// on whether we're connected or not.
HRESULT CDefaultProvider::GetCredentialAt(DWORD dwIndex, ICredentialProviderCredential** ppcpc)
{
    if (dwIndex != 0 || !ppcpc)
        return E_INVALIDARG;

    auto cred = getCredential();
    return cred->QueryInterface(IID_ICredentialProviderCredential, reinterpret_cast<void**>(ppcpc));
}
