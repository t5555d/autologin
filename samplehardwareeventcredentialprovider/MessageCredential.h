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
#include "fields.h"

enum SAMPLE_MESSAGE_FIELD_ID
{
    SMFI_TITLE,
    SMFI_MESSAGE
};

class CMessageCredential : public CBaseCredential
{
public: // ICredentialProviderCredential

    IFACEMETHODIMP GetSerialization(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE*,
        CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION*,
        PWSTR*, CREDENTIAL_PROVIDER_STATUS_ICON*) NOT_IMPLEMENTED;

public:

    CMessageCredential()
    {
        registerField(m_title);
        registerField(m_message);
    }

private:

    StringField m_title{ SMFI_TITLE, CPFT_LARGE_TEXT, CPFS_DISPLAY_IN_BOTH };
    StringField m_message{ SMFI_MESSAGE, CPFT_LARGE_TEXT, CPFS_DISPLAY_IN_SELECTED_TILE };
};
