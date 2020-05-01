#pragma once

#include "CBaseCredential.h"
#include "resource.h"

enum AUTO_FIELD_ID
{
    AUTO_IMAGE,
    AUTO_TITLE,
    AUTO_DOMAIN,
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
        registerField(imageField);
        registerField(titleField);
        registerField(domainField);
        registerField(usernameField);
        registerField(passwordField);
    }

    HRESULT Initialize(CREDENTIAL_PROVIDER_USAGE_SCENARIO usage);

    bool isAutoLogonEnabled() const { return autoLogon != 0; }

private:
    CREDENTIAL_PROVIDER_USAGE_SCENARIO usage; // The usage scenario for which we were enumerated.

    BOOL        autoLogon = FALSE;
    BitmapField imageField{ AUTO_IMAGE, MAKEINTRESOURCE(IDB_OPENED), CPFS_DISPLAY_IN_BOTH };
    StringField titleField{ AUTO_TITLE, CPFT_SMALL_TEXT, CPFS_DISPLAY_IN_BOTH, CPFIS_NONE };
    StringField domainField{ AUTO_DOMAIN, CPFT_EDIT_TEXT, CPFS_DISPLAY_IN_SELECTED_TILE, CPFIS_NONE, L"Domain Name" };
    StringField usernameField{ AUTO_USERNAME, CPFT_EDIT_TEXT, CPFS_DISPLAY_IN_SELECTED_TILE, CPFIS_NONE, L"User Name" };
    StringField passwordField{ AUTO_PASSWORD, CPFT_PASSWORD_TEXT, CPFS_DISPLAY_IN_SELECTED_TILE, CPFIS_FOCUSED, L"Password" };
};
