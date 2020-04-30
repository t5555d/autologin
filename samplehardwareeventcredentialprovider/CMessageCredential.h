#pragma once

#include "CBaseCredential.h"

enum MESSAGE_FIELD_ID
{
    MSG_TITLE,
    MSG_MESSAGE
};

class CMessageCredential : public CBaseCredential
{
public: // ICredentialProviderCredential

    HRESULT GetSerialization(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE*,
        CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION*,
        PWSTR*, CREDENTIAL_PROVIDER_STATUS_ICON*) NOT_IMPLEMENTED;

public:

    CMessageCredential()
    {
        registerField(titleField);
        registerField(messageField);
    }

private:

    StringField titleField{ MSG_TITLE, CPFT_LARGE_TEXT, CPFS_DISPLAY_IN_BOTH };
    StringField messageField{ MSG_MESSAGE, CPFT_LARGE_TEXT, CPFS_DISPLAY_IN_SELECTED_TILE };
};
