//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//
#include <unknwn.h>
#include "MessageCredential.h"
#include "guid.h"

HRESULT CMessageCredential::SetText(DWORD dwFieldId, LPCWSTR fmt, ...)
{
    if (dwFieldId >= SMFI_NUM_FIELDS)
        return E_INVALIDARG;

    va_list va;
    va_start(va, fmt);

    constexpr auto BUF_SIZE = 512;
    wchar_t buffer[BUF_SIZE];
    wsprintf(buffer, fmt, va);
    wvnsprintf(buffer, BUF_SIZE, fmt, va);
    va_end(va);

    m_fields[dwFieldId].SetValue(buffer);
    return S_OK;
}

HRESULT CMessageCredential::GetFieldDescriptorAt(
    DWORD dwIndex,
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** desc)
{
    if (dwIndex < SMFI_NUM_FIELDS)
        return FieldDescriptorCoAllocCopy(m_fields[dwIndex].GetDescriptor(), desc);
    else
        return E_INVALIDARG;
}

// Get info for a particular field of a tile. Called by logonUI to get information to 
// display the tile.
HRESULT CMessageCredential::GetFieldState(
    DWORD dwFieldID,
    CREDENTIAL_PROVIDER_FIELD_STATE* place,
    CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* state
    )
{
    if (dwFieldID < SMFI_NUM_FIELDS && place && state)
    {
        *place = m_fields[dwFieldID].GetPlace();
        *state = m_fields[dwFieldID].GetState();
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

// Called to request the string value of the indicated field.
HRESULT CMessageCredential::GetStringValue(DWORD dwFieldID, PWSTR* value)
{
    if (dwFieldID < SMFI_NUM_FIELDS && value)
    {
        // Make a copy of the string and return that. The caller
        // is responsible for freeing it.
        return SHStrDupW(m_fields[dwFieldID].GetValue(), value);
    }
    else
    {
        return E_INVALIDARG;
    }
}
