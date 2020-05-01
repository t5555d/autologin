#ifndef WIN32_NO_STATUS
#include <ntstatus.h>
#define WIN32_NO_STATUS
#endif

#include "helpers.h"

#include "CDefaultCredential.h"
#include "CDefaultProvider.h"

HRESULT getPrivatePassword(StringField& field)
{
    LSA_OBJECT_ATTRIBUTES attr{};
    LSA_HANDLE policy;

    NTSTATUS status = LsaOpenPolicy(NULL, &attr, POLICY_GET_PRIVATE_INFORMATION, &policy);
    if (status != STATUS_SUCCESS)
        return HRESULT_FROM_NT(status);

    WCHAR keyName[] = L"DefaultPassword";
    LSA_UNICODE_STRING key, *value = NULL;
    key.Buffer = keyName;
    key.Length = sizeof(keyName) - sizeof(WCHAR);
    key.MaximumLength = sizeof(keyName);
    status = LsaRetrievePrivateData(policy, &key, &value);

    if (status == STATUS_SUCCESS) {
        field.SetValue(value->Buffer, value->Length / sizeof(WCHAR));
        LsaFreeMemory(value);
    }

    LsaClose(policy);
    return HRESULT_FROM_NT(status);
}

HRESULT CDefaultCredential::Initialize(CREDENTIAL_PROVIDER_USAGE_SCENARIO value)
{
    usage = value;

    autoLogon = FALSE;
    HKEY key;
    LSTATUS error = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", 0, KEY_READ, &key);
    if (error == ERROR_SUCCESS)
    {
        wchar_t buffer[512];

        DWORD size = sizeof(buffer);
        error = RegQueryValueExW(key, L"AutoAdminLogon", nullptr, nullptr, (LPBYTE) buffer, &size);
        if (error == ERROR_SUCCESS) {
            auto intValue = wcstol(buffer, nullptr, 0);
            autoLogon = intValue ? TRUE : FALSE;
        }
    
        //if (autoLogon)
        {
            size = sizeof(buffer);
            error = RegQueryValueExW(key, L"DefaultDomainName", nullptr, nullptr, (LPBYTE)buffer, &size);
            if (error != ERROR_SUCCESS)
                error = GetComputerNameW(buffer, &size) ? ERROR_SUCCESS : GetLastError();

            if (error == ERROR_SUCCESS)
                domainField.SetValue(buffer, size);
            else
                autoLogon = FALSE;
        }

        //if (autoLogon)
        {
            size = sizeof(buffer);
            error = RegQueryValueExW(key, L"DefaultUserName", nullptr, nullptr, (LPBYTE)buffer, &size);
            if (error == ERROR_SUCCESS)
                usernameField.SetValue(buffer, size);
            else
                autoLogon = FALSE;
        }

        //if (autoLogon)
        {
            size = sizeof(buffer);
            error = RegQueryValueExW(key, L"DefaultPassword", nullptr, nullptr, (LPBYTE)buffer, &size);
            if (error == ERROR_SUCCESS)
                passwordField.SetValue(buffer, size);
            else
            {
                HRESULT res = getPrivatePassword(passwordField);
                if (FAILED(res))
                    autoLogon = FALSE;
            }
        }

        RegCloseKey(key);
    }

    return S_OK;
}

// LogonUI calls this function when our tile is selected (zoomed)
// If you simply want fields to show/hide based on the selected state,
// there's no need to do anything here - you can set that up in the 
// field definitions.  But if you want to do something
// more complicated, like change the contents of a field when the tile is
// selected, you would do it here.
HRESULT CDefaultCredential::SetSelected(BOOL* pbAutoLogon)  
{
    *pbAutoLogon = autoLogon;
    return S_OK;
}

// Collect the username and password into a serialized credential for the correct usage scenario. 
// LogonUI then passes these credentials back to the system to log on.
HRESULT CDefaultCredential::GetSerialization(
    CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
    CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, 
    PWSTR* ppwszOptionalStatusText, 
    CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon)
{
    UNREFERENCED_PARAMETER(ppwszOptionalStatusText);
    UNREFERENCED_PARAMETER(pcpsiOptionalStatusIcon);

    KERB_INTERACTIVE_LOGON kil;
    ZeroMemory(&kil, sizeof(kil));

    PWSTR domain = nullptr;
    PWSTR username = nullptr;
    PWSTR password = nullptr;
    KERB_INTERACTIVE_UNLOCK_LOGON kiul;

    HRESULT hr = SHStrDupW(usernameField.GetValue(), &username);

    if (SUCCEEDED(hr))
        hr = SHStrDupW(domainField.GetValue(), &domain);

    if (SUCCEEDED(hr))
        hr = ProtectIfNecessaryAndCopyPassword(passwordField.GetValue(), usage, &password);

    // Initialize kiul with weak references to our credential.
    if (SUCCEEDED(hr))
        hr = KerbInteractiveUnlockLogonInit(domain, username, password, usage, &kiul);

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
            pcpcs->clsidCredentialProvider = CDefaultProvider::CLSID;

            // At this point the credential has created the serialized credential used for logon
            // By setting this to CPGSR_RETURN_CREDENTIAL_FINISHED we are letting logonUI know
            // that we have all the information we need and it should attempt to submit the 
            // serialized credential.
            *pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED;
        }
    }

    CoTaskMemFree(domain);
    CoTaskMemFree(username);
    CoTaskMemFree(password);

    return hr;
}

struct REPORT_RESULT_STATUS_INFO
{
    NTSTATUS    ntsStatus;
    NTSTATUS    ntsSubstatus;
    PWSTR       pwzMessage;
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
HRESULT CDefaultCredential::ReportResult(
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
