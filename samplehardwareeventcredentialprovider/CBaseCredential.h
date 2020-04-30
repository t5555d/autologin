#pragma once

#include "common.h"
#include "dll.h"
#include "fields.h"
#include <vector>
#include <shlwapi.h>

#define NOT_IMPLEMENTED override { return E_NOTIMPL; }

class CBaseCredential : public ICredentialProviderCredential
{
    LONG _cRef;

public:
    CBaseCredential(): _cRef(1)
    {
        DllAddRef();
        m_fields.reserve(16);
    }
    virtual ~CBaseCredential() { DllRelease(); }

public: // IUnknown

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        return ++_cRef;
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        LONG cRef = --_cRef;
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(__in REFIID riid, __deref_out void** ppv) override
    {
        static const QITAB qit[] =
        {
            QITABENT(CBaseCredential, ICredentialProviderCredential), // IID_ICredentialProviderCredential
            {0},
        };
        return QISearch(this, qit, riid, ppv);
    }

public: // ICredentialProviderCredential default implementation for all field-related methods

    HRESULT GetFieldState(DWORD dwFieldID,
        CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
        CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis) override;

    HRESULT GetStringValue(DWORD, PWSTR*) override;
    HRESULT SetStringValue(DWORD, PCWSTR) override;

    HRESULT FormatStringValue(DWORD dwFieldId, PCWSTR fmt, ...);

    HRESULT GetBitmapValue(DWORD, HBITMAP*) NOT_IMPLEMENTED;
    HRESULT GetCheckboxValue(DWORD, BOOL*, PWSTR*) NOT_IMPLEMENTED;
    HRESULT GetComboBoxValueCount(DWORD, DWORD*, DWORD*) NOT_IMPLEMENTED;
    HRESULT GetComboBoxValueAt(DWORD, DWORD, PWSTR*) NOT_IMPLEMENTED;
    HRESULT GetSubmitButtonValue(DWORD, DWORD*) NOT_IMPLEMENTED;
    HRESULT SetCheckboxValue(DWORD, BOOL) NOT_IMPLEMENTED;
    HRESULT SetComboBoxSelectedValue(DWORD, DWORD) NOT_IMPLEMENTED;
    HRESULT CommandLinkClicked(DWORD) NOT_IMPLEMENTED;

public: // ICredentialProviderCredential default implementation for other optional methods

    HRESULT Advise(ICredentialProviderCredentialEvents*) NOT_IMPLEMENTED;
    HRESULT UnAdvise() NOT_IMPLEMENTED;

    HRESULT SetSelected(BOOL*) { return S_FALSE; }
    HRESULT SetDeselected() { return S_OK; }

    HRESULT ReportResult(NTSTATUS, NTSTATUS, PWSTR*,
        CREDENTIAL_PROVIDER_STATUS_ICON*) NOT_IMPLEMENTED;

public:

    HRESULT GetFieldDescriptorAt(DWORD dwIndex, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** desc);
    HRESULT GetFieldDescriptorCount(DWORD* pdwCount);
    
protected:
    void registerField(Field& field)
    {
        size_t min_size = field.GetID() + 1;
        if (m_fields.size() < min_size)
            m_fields.resize(min_size);
        m_fields[field.GetID()] = &field;
    }

    Field *getField(DWORD id) {
        if (id >= m_fields.size())
            return nullptr;
        return m_fields[id];
    }

private:
    std::vector<Field *> m_fields;
};