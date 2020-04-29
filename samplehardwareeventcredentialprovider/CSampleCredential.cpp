//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//

#ifndef WIN32_NO_STATUS
#include <ntstatus.h>
#define WIN32_NO_STATUS
#endif

#include "CSampleCredential.h"
#include "guid.h"


// Initializes one credential with the field information passed in.
// Set the value of the SFI_USERNAME field to pwzUsername.
HRESULT CSampleCredential::Initialize(CREDENTIAL_PROVIDER_USAGE_SCENARIO usage)
{
    m_usage = usage;

    m_fields[SFI_USERNAME].SetValue(L"testbot");
    m_fields[SFI_PASSWORD].SetValue(L"Test1234");

    return S_OK;
}

HRESULT CSampleCredential::GetFieldDescriptorAt(
    DWORD dwIndex,
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** desc)
{
    if (dwIndex < SFI_NUM_FIELDS)
        return FieldDescriptorCoAllocCopy(m_fields[dwIndex].GetDescriptor(), desc);
    else
        return E_INVALIDARG;
}



// LogonUI calls this function when our tile is selected (zoomed)
// If you simply want fields to show/hide based on the selected state,
// there's no need to do anything here - you can set that up in the 
// field definitions.  But if you want to do something
// more complicated, like change the contents of a field when the tile is
// selected, you would do it here.
HRESULT CSampleCredential::SetSelected(__out BOOL* pbAutoLogon)  
{
    *pbAutoLogon = TRUE;
    return S_OK;
}

// Get info for a particular field of a tile. Called by logonUI to get information to 
// display the tile.
HRESULT CSampleCredential::GetFieldState(
    DWORD dwFieldID,
    CREDENTIAL_PROVIDER_FIELD_STATE* place,
    CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* state
    )
{
    if (dwFieldID < SFI_NUM_FIELDS && place && state)
    {
        *state = m_fields[dwFieldID].GetState();
        *place = m_fields[dwFieldID].GetPlace();

        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

// Sets ppwsz to the string value of the field at the index dwFieldID.
HRESULT CSampleCredential::GetStringValue(
    DWORD dwFieldID, 
    __deref_out PWSTR* value
    )
{
    if (dwFieldID < SFI_NUM_FIELDS && value)
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

// Collect the username and password into a serialized credential for the correct usage scenario 
// (logon/unlock is what's demonstrated in this sample).  LogonUI then passes these credentials 
// back to the system to log on.
HRESULT CSampleCredential::GetSerialization(
    CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
    CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, 
    PWSTR* ppwszOptionalStatusText, 
    CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon)
{
    UNREFERENCED_PARAMETER(ppwszOptionalStatusText);
    UNREFERENCED_PARAMETER(pcpsiOptionalStatusIcon);

    KERB_INTERACTIVE_LOGON kil;
    ZeroMemory(&kil, sizeof(kil));

    WCHAR domain[MAX_COMPUTERNAME_LENGTH+1];
    DWORD cch = ARRAYSIZE(domain);
    if (!GetComputerNameW(domain, &cch))
    {
        DWORD dwErr = GetLastError();
        return HRESULT_FROM_WIN32(dwErr);
    }

    PWSTR username = nullptr;
    PWSTR password = nullptr;
    KERB_INTERACTIVE_UNLOCK_LOGON kiul;

    HRESULT hr = SHStrDupW(m_fields[SFI_USERNAME].GetValue(), &username);

    if (SUCCEEDED(hr))
        hr = ProtectIfNecessaryAndCopyPassword(m_fields[SFI_PASSWORD].GetValue(), m_usage, &password);

    // Initialize kiul with weak references to our credential.
    if (SUCCEEDED(hr))
        hr = KerbInteractiveUnlockLogonInit(domain, username, password, m_usage, &kiul);

    // We use KERB_INTERACTIVE_UNLOCK_LOGON in both unlock and logon scenarios.  It contains a
    // KERB_INTERACTIVE_LOGON to hold the creds plus a LUID that is filled in for us by Winlogon
    // as necessary.
    if (SUCCEEDED(hr))
        hr = KerbInteractiveUnlockLogonPack(kiul, &pcpcs->rgbSerialization, &pcpcs->cbSerialization);

    if (SUCCEEDED(hr))
    {
        ULONG ulAuthPackage;
        hr = RetrieveNegotiateAuthPackage(&ulAuthPackage);
        if (SUCCEEDED(hr))
        {
            pcpcs->ulAuthenticationPackage = ulAuthPackage;
            pcpcs->clsidCredentialProvider = CLSID_CSample;

            // At this point the credential has created the serialized credential used for logon
            // By setting this to CPGSR_RETURN_CREDENTIAL_FINISHED we are letting logonUI know
            // that we have all the information we need and it should attempt to submit the 
            // serialized credential.
            *pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED;
        }
    }

    CoTaskMemFree(username);
    CoTaskMemFree(password);

    return hr;
}

struct REPORT_RESULT_STATUS_INFO
{
    NTSTATUS ntsStatus;
    NTSTATUS ntsSubstatus;
    PWSTR     pwzMessage;
    CREDENTIAL_PROVIDER_STATUS_ICON cpsi;
};

static const REPORT_RESULT_STATUS_INFO s_rgLogonStatusInfo[] =
{
    { STATUS_LOGON_FAILURE, STATUS_SUCCESS, L"Incorrect password or username.", CPSI_ERROR, },
    { STATUS_ACCOUNT_RESTRICTION, STATUS_ACCOUNT_DISABLED, L"The account is disabled.", CPSI_WARNING },
};

// ReportResult is completely optional.  Its purpose is to allow a credential to customize the string
// and the icon displayed in the case of a logon failure.  For example, we have chosen to 
// customize the error shown in the case of bad username/password and in the case of the account
// being disabled.
HRESULT CSampleCredential::ReportResult(
    NTSTATUS ntsStatus, 
    NTSTATUS ntsSubstatus,
    PWSTR* ppwszOptionalStatusText, 
    CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon
    )
{
    *ppwszOptionalStatusText = NULL;
    *pcpsiOptionalStatusIcon = CPSI_NONE;

    DWORD dwStatusInfo = (DWORD)-1;

    // Look for a match on status and substatus.
    for (DWORD i = 0; i < ARRAYSIZE(s_rgLogonStatusInfo); i++)
    {
        if (s_rgLogonStatusInfo[i].ntsStatus == ntsStatus && s_rgLogonStatusInfo[i].ntsSubstatus == ntsSubstatus)
        {
            dwStatusInfo = i;
            break;
        }
    }

    if ((DWORD)-1 != dwStatusInfo)
    {
        if (SUCCEEDED(SHStrDupW(s_rgLogonStatusInfo[dwStatusInfo].pwzMessage, ppwszOptionalStatusText)))
        {
            *pcpsiOptionalStatusIcon = s_rgLogonStatusInfo[dwStatusInfo].cpsi;
        }
    }

    // Since NULL is a valid value for *ppwszOptionalStatusText and *pcpsiOptionalStatusIcon
    // this function can't fail.
    return S_OK;
}
