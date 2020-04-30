#pragma once

#include "CBaseCredential.h"

enum SAMPLE_FIELD_ID
{
    SFI_TITLE,
    SFI_USERNAME,
    SFI_PASSWORD
};

class CSampleCredential : public CBaseCredential
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
    HRESULT Initialize(CREDENTIAL_PROVIDER_USAGE_SCENARIO usage);

    CSampleCredential()
    {
        registerField(m_title);
        registerField(m_username);
        registerField(m_password);
    }

private:
    CREDENTIAL_PROVIDER_USAGE_SCENARIO    m_usage; // The usage scenario for which we were enumerated.

    StringField m_title{ SFI_TITLE, CPFT_SMALL_TEXT, CPFS_DISPLAY_IN_BOTH, CPFIS_NONE };
    StringField m_username{ SFI_USERNAME, CPFT_LARGE_TEXT, CPFS_DISPLAY_IN_SELECTED_TILE, CPFIS_NONE };
    StringField m_password{ SFI_PASSWORD, CPFT_PASSWORD_TEXT, CPFS_DISPLAY_IN_SELECTED_TILE, CPFIS_FOCUSED };
};
