#include "RDPState.h"
#include <stdexcept>

void RDPState::start(int interval)
{
    if (m_interval == 0) {
        m_interval = interval;
        std::thread new_thread(&RDPState::watch, this);
        m_thread.swap(new_thread);
    }
    else {
        set_interval(interval);
    }
}

void RDPState::stop()
{
    m_interval = 0;
    if (m_thread.joinable())
        m_thread.join();
}

void RDPState::watch()
{
    const auto RDP_PORT = htons(3389);

    while (m_interval > 0) {
        auto res = GetTcpTable(m_table, &m_table_size, true);
        if (res == ERROR_INSUFFICIENT_BUFFER) {
            m_table = (decltype(m_table))realloc(m_table, m_table_size);
            continue;
        }

        if (res == NO_ERROR) {

            auto *table = m_table->table;
            auto count = m_table->dwNumEntries;

            DWORD rdp_peer = 0;
            for (DWORD i = 0; i < count; i++) {
                if (table[i].dwLocalPort == RDP_PORT && table[i].State == MIB_TCP_STATE_ESTAB) {
                    rdp_peer = table[i].dwRemoteAddr;
                    break;
                }
            }
            if (rdp_peer != m_rdp_peer) {
                m_rdp_peer = rdp_peer;

                snprintf(m_rdp_text, sizeof(m_rdp_text), "%d.%d.%d.%d",
                    rdp_peer & 0xFF, (rdp_peer >> 8) & 0xFF, (rdp_peer >> 16) & 0xFF, (rdp_peer >> 24) & 0xFF);

                if (m_callback)
                    m_callback(m_context);
            }
        }

        Sleep(m_interval);
    }
}

