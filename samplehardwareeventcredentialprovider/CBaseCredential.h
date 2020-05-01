#pragma once

#include <vector>
#include <string>
#include <credentialprovider.h>
#include "CUnknown.h"

class Field
{
public:

    using TYPE = CREDENTIAL_PROVIDER_FIELD_TYPE;
    using PLACE = CREDENTIAL_PROVIDER_FIELD_STATE;
    using STATE = CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE;

    static constexpr PLACE DEFAULT_PLACE = CPFS_DISPLAY_IN_SELECTED_TILE;
    static constexpr STATE DEFAULT_STATE = CPFIS_NONE;

    DWORD   GetID() const { return id; }
    TYPE    GetType() const { return type; }
    PLACE   GetPlace() const { return place; }
    STATE   GetState() const { return state; }

protected:

    Field(DWORD id, TYPE type, PLACE place = DEFAULT_PLACE, STATE state = DEFAULT_STATE) :
        id(id), type(type), place(place), state(state) {}
    virtual ~Field() {}

    const DWORD     id;
    const TYPE      type;
    PLACE           place;
    STATE           state;
};

class StringField : public Field
{
public:

    static constexpr TYPE DEFAULT_TYPE = CPFT_LARGE_TEXT;

    StringField(DWORD id, TYPE type = DEFAULT_TYPE, PLACE place = DEFAULT_PLACE, STATE state = DEFAULT_STATE) :
        Field(id, type, place, state) {}
    ~StringField() { clean(); }

    LPCWSTR GetValue() const { return value.c_str(); }

    void SetValue(LPCWSTR text, size_t count = 0)
    {
        if (type == CPFT_PASSWORD_TEXT)
            clean();
        if (count)
            value.assign(text, count);
        else
            value.assign(text);
    }

private:
    std::wstring value;

    void clean();
};

class CBaseCredential : public CUnknown<ICredentialProviderCredential>
{
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
        if (fields.size() < min_size)
            fields.resize(min_size);
        fields[field.GetID()] = &field;
    }

    Field *getField(DWORD id)
    {
        if (id >= fields.size())
            return nullptr;
        return fields[id];
    }

private:
    std::vector<Field *> fields;
};