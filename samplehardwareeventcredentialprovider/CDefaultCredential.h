#pragma once

#include "CBaseCredential.h"

enum AUTO_FIELD_ID
{
    AUTO_TITLE,
    AUTO_USERNAME,
    AUTO_PASSWORD
};

class CDefaultCredential : public CBaseCredential
{
public: // ICredentialProviderCredential

    HRESULT SetSelected(BOOL* pbAutoLogon) override;

    HRESULT GetSerialization(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, 
                                    CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, 
                                    PWSTR* ppwszOptionalStatusText, 
                                    CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon) override;
    HRESULT ReportResult(NTSTATUS ntsStatus, 
                                NTSTATUS ntsSubstatus,
                                PWSTR* ppwszOptionalStatusText, 
                                CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon) override;

public:

    CDefaultCredential()
    {
        registerField(titleField);
        registerField(usernameField);
        registerField(passwordField);
    }

    HRESULT SetUsage(CREDENTIAL_PROVIDER_USAGE_SCENARIO usage);

private:
    CREDENTIAL_PROVIDER_USAGE_SCENARIO usage; // The usage scenario for which we were enumerated.

    StringField titleField{ AUTO_TITLE, CPFT_SMALL_TEXT, CPFS_DISPLAY_IN_BOTH, CPFIS_NONE };
    StringField usernameField{ AUTO_USERNAME, CPFT_LARGE_TEXT, CPFS_DISPLAY_IN_SELECTED_TILE, CPFIS_NONE };
    StringField passwordField{ AUTO_PASSWORD, CPFT_PASSWORD_TEXT, CPFS_DISPLAY_IN_SELECTED_TILE, CPFIS_FOCUSED };
};
