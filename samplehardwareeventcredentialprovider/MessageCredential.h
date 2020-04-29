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
#include "common.h"
#include "dll.h"

class CMessageCredential : public ICredentialProviderCredential
{
    public:
    // IUnknown
    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return ++_cRef;
    }
    
    IFACEMETHODIMP_(ULONG) Release()
    {
        LONG cRef = --_cRef;
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    IFACEMETHODIMP QueryInterface(__in REFIID riid, __deref_out void** ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CMessageCredential, ICredentialProviderCredential), // IID_ICredentialProviderCredential
            {0},
        };
        return QISearch(this, qit, riid, ppv);
    }

public: // ICredentialProviderCredential

    IFACEMETHODIMP GetFieldState(__in DWORD dwFieldID,
        __out CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
        __out CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis);

    IFACEMETHODIMP GetStringValue(__in DWORD dwFieldID, __deref_out PWSTR* ppwsz);

public: // ICredentialProviderCredential unused methods

    IFACEMETHODIMP Advise(ICredentialProviderCredentialEvents*) { return E_NOTIMPL; }
    IFACEMETHODIMP UnAdvise() { return E_NOTIMPL; }

    IFACEMETHODIMP SetSelected(BOOL*) { return S_FALSE; }
    IFACEMETHODIMP SetDeselected() { return S_OK; }

    IFACEMETHODIMP GetSerialization(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE*, 
                                    CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION*, 
                                    PWSTR*, CREDENTIAL_PROVIDER_STATUS_ICON*) { return E_NOTIMPL; }
    IFACEMETHODIMP ReportResult(NTSTATUS, NTSTATUS, PWSTR*, 
        CREDENTIAL_PROVIDER_STATUS_ICON*) { return E_NOTIMPL; }

public: // ICredentialProviderCredential unused methods
        // not required, since our tile doesn't have such fields

    IFACEMETHODIMP GetBitmapValue(DWORD, HBITMAP*) { return E_NOTIMPL; }
    IFACEMETHODIMP GetCheckboxValue(DWORD, BOOL*, PWSTR*) { return E_NOTIMPL; }
    IFACEMETHODIMP GetComboBoxValueCount(DWORD, DWORD*, DWORD*) { return E_NOTIMPL; }
    IFACEMETHODIMP GetComboBoxValueAt(DWORD, DWORD, PWSTR*) { return E_NOTIMPL; }
    IFACEMETHODIMP GetSubmitButtonValue(DWORD, DWORD*) { return E_NOTIMPL; }
    IFACEMETHODIMP SetStringValue(DWORD, PCWSTR) { return E_NOTIMPL; }
    IFACEMETHODIMP SetCheckboxValue(DWORD, BOOL) { return E_NOTIMPL; }
    IFACEMETHODIMP SetComboBoxSelectedValue(DWORD, DWORD) { return E_NOTIMPL; }
    IFACEMETHODIMP CommandLinkClicked(DWORD) { return E_NOTIMPL; }

public:
    HRESULT Initialize(__in const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgcpfd,
                       __in const FIELD_STATE_PAIR* rgfsp);

    HRESULT SetMessage(LPCWSTR fmt, ...);

    CMessageCredential();

    virtual ~CMessageCredential();

  private:
    LONG                                    _cRef;
    
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR    _rgCredProvFieldDescriptors[SMFI_NUM_FIELDS];   // An array holding the 
                                                                                            // type and name of each 
                                                                                            // field in the tile.
    
    FIELD_STATE_PAIR                        _rgFieldStatePairs[SMFI_NUM_FIELDS];            // An array holding the 
                                                                                            // state of each field in 
                                                                                            // the tile.

    PWSTR                                   _rgFieldStrings[SMFI_NUM_FIELDS];               // An array holding the 
                                                                                            // string value of each 
                                                                                            // field. This is different 
                                                                                            // from the name of the 
                                                                                            // field held in 
                                                                                            // _rgCredProvFieldDescriptors.

};
