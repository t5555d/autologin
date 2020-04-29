//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//

#pragma once

#include <windows.h>
#include <strsafe.h>
#include <shlguid.h>
#include "helpers.h"
#include "CBaseCredential.h"

// Same as SAMPLE_FIELD_ID above, but for the CMessageCredential.
enum SAMPLE_MESSAGE_FIELD_ID
{
    SMFI_TITLE,
    SMFI_MESSAGE,
    SMFI_NUM_FIELDS
};

// Same as s_rgFieldStatePairs above, but for the CMessageCredential.
static const FIELD_STATE_PAIR s_rgMessageFieldStatePairs[] =
{
    { CPFS_DISPLAY_IN_BOTH, CPFIS_NONE },                   // SMFI_TITLE
    { CPFS_DISPLAY_IN_SELECTED_TILE, CPFIS_NONE }           // SMFI_MESSAGE
};

// Same as s_rgCredProvFieldDescriptors above, but for the CMessageCredential.
static const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR s_rgMessageCredProvFieldDescriptors[] =
{
    { SMFI_TITLE, CPFT_LARGE_TEXT, nullptr },
    { SMFI_MESSAGE, CPFT_LARGE_TEXT, nullptr },
};

class CMessageCredential : public CBaseCredential
{
public: // ICredentialProviderCredential

    IFACEMETHODIMP GetFieldState(DWORD dwFieldID,
        CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
        CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis) override;

    IFACEMETHODIMP GetStringValue(DWORD dwFieldID, PWSTR* ppwsz) override;

    IFACEMETHODIMP GetSerialization(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE*,
        CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION*,
        PWSTR*, CREDENTIAL_PROVIDER_STATUS_ICON*) NOT_IMPLEMENTED;

public:
    HRESULT Initialize(const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgcpfd,
                       const FIELD_STATE_PAIR* rgfsp);

    HRESULT SetMessage(LPCWSTR fmt, ...);

    CMessageCredential() = default;

    virtual ~CMessageCredential();

private:
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR    _rgCredProvFieldDescriptors[SMFI_NUM_FIELDS]{}; // An array holding the 
                                                                                            // type and name of each 
                                                                                            // field in the tile.
    
    FIELD_STATE_PAIR                        _rgFieldStatePairs[SMFI_NUM_FIELDS]{};          // An array holding the 
                                                                                            // state of each field in 
                                                                                            // the tile.

    PWSTR                                   _rgFieldStrings[SMFI_NUM_FIELDS]{};             // An array holding the 
                                                                                            // string value of each 
                                                                                            // field. This is different 
                                                                                            // from the name of the 
                                                                                            // field held in 
                                                                                            // _rgCredProvFieldDescriptors.
};
