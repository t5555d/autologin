#pragma once
#include <windows.h>
#include <iphlpapi.h>
#include <thread>
#include <atomic>

class RDPState
{
public:
    ~RDPState() { stop(); }

    bool isActive() const { return peerAddr != 0; }
    DWORD getPeerAddr() const { return peerAddr; }
    DWORD getPeerPort() const { return peerPort; }

    const char *getPeerText() const {
        return peerAddr ? peerText : nullptr;
    }

    void setInterval(int value) {
        if (value > 0)
            interval = value;
    }

    template<typename Context>
    void setCallback(void (*func)(Context *), Context *ctx) {
        callback = reinterpret_cast<callback_t>(func);
        context = ctx;
    }

    void start(int interval = 50);
    void stop();

private:
    using callback_t = void(*)(void *context);

    MIB_TCPTABLE *  table = nullptr;
    DWORD           tableSize = 0;
    DWORD           peerAddr = 0;
    DWORD           peerPort = 0;
    char            peerText[16];
    std::atomic_int interval = 0;
    std::thread     thread;
    callback_t      callback = nullptr;
    void *          context = nullptr;

    void watch();
};

