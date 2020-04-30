#include "CBaseCredential.h"
#include "helpers.h"

void StringField::clean()
{
    auto raw = const_cast<wchar_t *>(value.c_str());
    auto len = value.capacity() * sizeof(wchar_t);
    SecureZeroMemory(raw, len);
}

HRESULT CBaseCredential::GetFieldDescriptorCount(DWORD* count)
{
    *count = (DWORD) fields.size();
    return S_OK;
}

HRESULT CBaseCredential::GetFieldDescriptorAt(
    DWORD dwFieldID,
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** pdesc)
{
    auto field = getField(dwFieldID);
    if (!field)
        return E_INVALIDARG;

    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR desc = { 0 };
    desc.dwFieldID = field->GetID();
    desc.cpft = field->GetType();

    return FieldDescriptorCoAllocCopy(desc, pdesc);
}

// Get info for a particular field of a tile. Called by logonUI to get information to 
// display the tile.
HRESULT CBaseCredential::GetFieldState(
    DWORD dwFieldID,
    CREDENTIAL_PROVIDER_FIELD_STATE* place,
    CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* state
)
{
    auto field = getField(dwFieldID);
    if (!field || !place || !state)
        return E_INVALIDARG;

    *place = field->GetPlace();
    *state = field->GetState();
    return S_OK;
}

HRESULT CBaseCredential::GetStringValue(DWORD dwFieldID, PWSTR* value)
{
    auto field = getField(dwFieldID);
    if (!field || !value)
        return E_INVALIDARG;

    auto string = dynamic_cast<StringField *>(field);
    if (!string)
        return E_NOINTERFACE;

    // Make a copy of the string and return that. The caller
    // is responsible for freeing it.
    return SHStrDupW(string->GetValue(), value);
}

HRESULT CBaseCredential::SetStringValue(DWORD dwFieldID, PCWSTR value)
{
    auto field = getField(dwFieldID);
    if (!field || !value)
        return E_INVALIDARG;

    auto string = dynamic_cast<StringField *>(field);
    if (!string)
        return E_NOINTERFACE;

    string->SetValue(value);
    return S_OK;
}

HRESULT CBaseCredential::FormatStringValue(DWORD dwFieldID, PCWSTR format, ...)
{
    auto field = getField(dwFieldID);
    if (!field || !format)
        return E_INVALIDARG;

    auto string = dynamic_cast<StringField *>(field);
    if (!string)
        return E_NOINTERFACE;

    va_list va;
    va_start(va, format);

    constexpr auto BUF_SIZE = 512;
    wchar_t value[BUF_SIZE];
    wsprintf(value, format, va);
    wvnsprintf(value, BUF_SIZE, format, va);
    va_end(va);

    string->SetValue(value);
    return S_OK;
}
