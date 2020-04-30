#pragma once
#include <stdexcept>

void DllAddRef();
void DllRelease();

template<typename C>
HRESULT CreateInstance(REFIID riid, void **ppv)
{
    try {
        C* c = new C();
        HRESULT hr = c->QueryInterface(riid, ppv);
        c->Release();
        return hr;
    }
    catch (const std::bad_alloc&) {
        return E_OUTOFMEMORY;
    }
}