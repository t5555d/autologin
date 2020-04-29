#pragma once
#include <windows.h>
#include <iphlpapi.h>
#include <thread>
#include <atomic>

class RDPState
{
public:
    ~RDPState() { stop(); }

    bool is_rdp_active() const { return m_rdp_peer != 0; }
    DWORD get_rdp_addr() const { return m_rdp_peer; }

    const char *get_rdp_text() const {
        return m_rdp_peer ? m_rdp_text : nullptr;
    }

    void set_interval(int interval) {
        if (interval > 0) 
            m_interval = interval;
    }

    template<typename Context>
    void set_callback(void (*callback)(Context *), Context *context) {
        m_callback = reinterpret_cast<callback_t>(callback);
        m_context = context;
    }

    void start(int interval = 50);
    void stop();

private:
    using callback_t = void(*)(void *context);

    MIB_TCPTABLE *  m_table = nullptr;
    DWORD           m_table_size = 0;
    DWORD           m_rdp_peer = 0;
    char            m_rdp_text[16];
    std::atomic_int m_interval = 0;
    std::thread     m_thread;
    callback_t      m_callback = nullptr;
    void *          m_context = nullptr;

    void watch();
};

