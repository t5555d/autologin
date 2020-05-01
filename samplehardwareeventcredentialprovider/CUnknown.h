#pragma once

#include <atomic>
#include <stdexcept>
#include <shlwapi.h>

#define NOT_IMPLEMENTED override { return E_NOTIMPL; }

extern HINSTANCE g_instance;

void DllAddRef();
void DllRelease();

template<typename Base>
class CUnknown : public Base // Base should inherit from IUnknown
{
public: // IUnknown

    ULONG AddRef() override
    {
        return ++_cRef;
    }

    ULONG Release() override
    {
        auto cRef = --_cRef;
        if (!cRef)
            delete this;
        return cRef;
    }

    HRESULT QueryInterface(REFIID riid, void** ppv) override
    {
        static const QITAB qit[] =
        {
            QITABENT(CUnknown, Base),
            {0},
        };
        return QISearch(this, qit, riid, ppv);
    }

protected:

    CUnknown() : _cRef(1)
    {
        DllAddRef();
    }

    virtual ~CUnknown()
    {
        DllRelease();
    }

private:
    std::atomic<LONG> _cRef;
};

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
