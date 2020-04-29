#include "fields.h"

CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR Field::GetDescriptor() const
{
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR desc = { 0 };
    desc.dwFieldID = m_id;
    desc.cpft = m_type;
    return desc;
}

void StringField::clean()
{
    auto raw = const_cast<wchar_t *>(m_value.c_str());
    auto len = m_value.capacity() * sizeof(wchar_t);
    SecureZeroMemory(raw, len);
}
