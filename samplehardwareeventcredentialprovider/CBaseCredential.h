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
    LPCWSTR GetLabel() const { return label; }

protected:

    Field(DWORD id, TYPE type, PLACE place = DEFAULT_PLACE, STATE state = DEFAULT_STATE, LPCWSTR label = nullptr) :
        id(id), type(type), place(place), state(state), label(label) {}
    virtual ~Field() {}

    const DWORD     id;
    const TYPE      type;
    PLACE           place;
    STATE           state;
    LPCWSTR         label;
};

class StringField : public Field
{
public:

    static constexpr TYPE DEFAULT_TYPE = CPFT_LARGE_TEXT;

    StringField(DWORD id, TYPE type = DEFAULT_TYPE, PLACE place = DEFAULT_PLACE, STATE state = DEFAULT_STATE, LPCWSTR label = nullptr) :
        Field(id, type, place, state, label) {}
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

class BitmapField : public Field
{
public:
    static constexpr TYPE REQUIRED_TYPE = CPFT_TILE_IMAGE;

    BitmapField(DWORD id, LPCWSTR bitmapName, PLACE place = DEFAULT_PLACE, STATE state = DEFAULT_STATE) :
        Field(id, REQUIRED_TYPE, place, state)
    {
        value = (HBITMAP) LoadImage(g_instance, bitmapName, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_SHARED | LR_LOADTRANSPARENT);
    }

    HBITMAP GetValue() const { return value; }

private:
    HBITMAP value;
};

class CBaseCredential : public CUnknown<ICredentialProviderCredential>
{
public: // ICredentialProviderCredential default implementation for all field-related methods

    HRESULT STDCALL GetFieldState(DWORD dwFieldID,
        CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
        CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis) override;

    HRESULT STDCALL GetStringValue(DWORD, PWSTR*) override;
    HRESULT STDCALL SetStringValue(DWORD, PCWSTR) override;
    HRESULT STDCALL GetBitmapValue(DWORD, HBITMAP*) override;
    HRESULT STDCALL GetCheckboxValue(DWORD, BOOL*, PWSTR*) NOT_IMPLEMENTED;
    HRESULT STDCALL GetComboBoxValueCount(DWORD, DWORD*, DWORD*) NOT_IMPLEMENTED;
    HRESULT STDCALL GetComboBoxValueAt(DWORD, DWORD, PWSTR*) NOT_IMPLEMENTED;
    HRESULT STDCALL GetSubmitButtonValue(DWORD, DWORD*) NOT_IMPLEMENTED;
    HRESULT STDCALL SetCheckboxValue(DWORD, BOOL) NOT_IMPLEMENTED;
    HRESULT STDCALL SetComboBoxSelectedValue(DWORD, DWORD) NOT_IMPLEMENTED;
    HRESULT STDCALL CommandLinkClicked(DWORD) NOT_IMPLEMENTED;

public: // ICredentialProviderCredential default implementation for other optional methods

    HRESULT STDCALL Advise(ICredentialProviderCredentialEvents*) NOT_IMPLEMENTED;
    HRESULT STDCALL UnAdvise() NOT_IMPLEMENTED;

    HRESULT STDCALL SetSelected(BOOL*) { return S_FALSE; }
    HRESULT STDCALL SetDeselected() { return S_OK; }

    HRESULT STDCALL ReportResult(NTSTATUS, NTSTATUS, PWSTR*,
        CREDENTIAL_PROVIDER_STATUS_ICON*) NOT_IMPLEMENTED;

public:

    HRESULT GetFieldDescriptorAt(DWORD dwIndex, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** desc);
    HRESULT GetFieldDescriptorCount(DWORD* pdwCount);

    HRESULT FormatStringValue(DWORD dwFieldId, PCWSTR fmt, ...);

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