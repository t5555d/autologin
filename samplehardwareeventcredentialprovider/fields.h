#pragma once

#include "common.h"
#include <string>

class Field
{
public:

    using TYPE = CREDENTIAL_PROVIDER_FIELD_TYPE;
    using PLACE = CREDENTIAL_PROVIDER_FIELD_STATE;
    using STATE = CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE;

    static constexpr PLACE DEFAULT_PLACE = CPFS_DISPLAY_IN_SELECTED_TILE;
    static constexpr STATE DEFAULT_STATE = CPFIS_NONE;

    Field(DWORD id, TYPE type, PLACE place = DEFAULT_PLACE, STATE state = DEFAULT_STATE) :
        m_id(id), m_type(type), m_place(place), m_state(state) {}

    DWORD   GetID() const { return m_id; }
    TYPE    GetType() const { return m_type; }
    PLACE   GetPlace() const { return m_place; }
    STATE   GetState() const { return m_state; }

    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR GetDescriptor() const;

protected:
    const DWORD     m_id;
    const TYPE      m_type;
    PLACE           m_place;
    STATE           m_state;
    LPCWSTR         m_label;
};

class StringField : public Field
{
public:

    static constexpr TYPE DEFAULT_TYPE = CPFT_LARGE_TEXT;

    StringField(DWORD id, TYPE type = DEFAULT_TYPE, PLACE place = DEFAULT_PLACE, STATE state = DEFAULT_STATE) :
        Field(id, type, place, state) {}
    ~StringField() { clean(); }

    LPCWSTR GetValue() const { return m_value.c_str(); }

    void SetValue(LPCWSTR value)
    {
        if (m_type == CPFT_PASSWORD_TEXT)
            clean();
        m_value = value;
    }

private:
    std::wstring m_value;

    void clean();
};
