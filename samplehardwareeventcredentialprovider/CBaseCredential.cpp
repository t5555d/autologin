#include "CBaseCredential.h"
#include "helpers.h"

HRESULT CBaseCredential::GetFieldDescriptorAt(
    DWORD dwIndex,
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** desc)
{
    if (dwIndex < m_fields.size())
        return FieldDescriptorCoAllocCopy(m_fields[dwIndex]->GetDescriptor(), desc);
    else
        return E_INVALIDARG;
}

// Get info for a particular field of a tile. Called by logonUI to get information to 
// display the tile.
HRESULT CBaseCredential::GetFieldState(
    DWORD dwFieldID,
    CREDENTIAL_PROVIDER_FIELD_STATE* place,
    CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* state
)
{
    if (dwFieldID >= m_fields.size() || !place || !state)
        return E_INVALIDARG;

    *place = m_fields[dwFieldID]->GetPlace();
    *state = m_fields[dwFieldID]->GetState();
    return S_OK;
}


HRESULT CBaseCredential::GetStringValue(DWORD dwFieldID, PWSTR* value)
{
    if (dwFieldID >= m_fields.size() || !value)
        return E_INVALIDARG;

    auto string = static_cast<StringField *>(m_fields[dwFieldID]);
    if (!string)
        return E_NOINTERFACE;

    // Make a copy of the string and return that. The caller
    // is responsible for freeing it.
    return SHStrDupW(string->GetValue(), value);
}

HRESULT CBaseCredential::SetStringValue(DWORD dwFieldID, PCWSTR value)
{
    if (dwFieldID >= m_fields.size() || !value)
        return E_INVALIDARG;

    auto string = static_cast<StringField *>(m_fields[dwFieldID]);
    if (!string)
        return E_NOINTERFACE;

    string->SetValue(value);
    return S_OK;
}

HRESULT CBaseCredential::FormatStringValue(DWORD dwFieldID, PCWSTR format, ...)
{
    if (dwFieldID >= m_fields.size() || !format)
        return E_INVALIDARG;

    va_list va;
    va_start(va, format);

    constexpr auto BUF_SIZE = 512;
    wchar_t value[BUF_SIZE];
    wsprintf(value, format, va);
    wvnsprintf(value, BUF_SIZE, format, va);
    va_end(va);

    return SetStringValue(dwFieldID, value);
}
