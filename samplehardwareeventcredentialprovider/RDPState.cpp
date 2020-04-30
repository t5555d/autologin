#include "RDPState.h"
#include <stdexcept>

void RDPState::start(int watchInterval)
{
    if (interval == 0) {
        interval = watchInterval;
        std::thread new_thread(&RDPState::watch, this);
        thread.swap(new_thread);
    }
    else {
        setInterval(interval);
    }
}

void RDPState::stop()
{
    interval = 0;
    if (thread.joinable())
        thread.join();
}

static WORD getRDPPort()
{
    DWORD port = 3389, size = sizeof(port);
    HKEY key;
    LONG error = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\WinStations\\RDP-Tcp", 0, KEY_READ, &key);
    if (error == ERROR_SUCCESS) {
        RegQueryValueExA(key, "PortNumber", 0, NULL, reinterpret_cast<LPBYTE>(&port), &size);
        RegCloseKey(key);
    }
    return (WORD) port;
}

void RDPState::watch()
{
    const auto RDP_PORT = htons(getRDPPort());

    while (interval > 0) {
        auto res = GetTcpTable(table, &tableSize, true);
        if (res == ERROR_INSUFFICIENT_BUFFER) {
            table = (decltype(table))realloc(table, tableSize);
            continue;
        }

        if (res == NO_ERROR) {

            auto tcp_table = table->table;
            auto row_count = table->dwNumEntries;

            DWORD new_addr = 0, new_port = 0;
            for (DWORD i = 0; i < row_count; i++) {
                if (tcp_table[i].dwLocalPort == RDP_PORT && tcp_table[i].dwState == MIB_TCP_STATE_ESTAB) {
                    new_addr = tcp_table[i].dwRemoteAddr;
                    new_port = tcp_table[i].dwRemotePort;
                    break;
                }
            }
            if (new_addr != peerAddr || new_port != peerPort) {
                peerAddr = new_addr;
                peerPort = new_port;

                snprintf(peerText, sizeof(peerText), "%d.%d.%d.%d",
                    peerAddr & 0xFF, (peerAddr >> 8) & 0xFF, (peerAddr >> 16) & 0xFF, (peerAddr >> 24) & 0xFF);

                if (callback)
                    callback(context);
            }
        }

        Sleep(interval);
    }
}

