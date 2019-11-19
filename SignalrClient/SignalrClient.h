#pragma once

#include "SignalrCommon.h"
#include "../Common/Lockable.h"
#include "ISignalrClient.h"
#include <thread>
#include <concurrent_queue.h>
#include <signalrclient/log_writer.h>
#include <signalrclient/hub_connection.h>

class logger final : public signalr::log_writer {
    void __cdecl write(const std::string& entry) override {
        logTSMessage(entry.c_str());
    }
};

class SignalrClient final : public ISignalrClient, public Lockable {
public:
    SignalrClient() = default;
    ~SignalrClient();
    void initialize(std::function<void()> connectedCallback, std::function<void(ClientMessage)> procedureCallback) override;
    void shutdown() override;
    void sendMessage(SERVER_MESSAGE_TYPE procedure, signalr::value value) override;

DECLARE_MEMBER(bool, ClientConnecting)
DECLARE_MEMBER(bool, ClientConnected)
DECLARE_MEMBER(bool, ShuttingDown)

    bool getConnected() override {
        return this->getClientConnected();
    }

    void setConnected(const bool value) override {
        this->setClientConnected(value);
    }

private:
    std::function<void()> m_connectedCallback;
    std::function<void(ClientMessage)> m_procedureCallback;
    std::shared_ptr<signalr::hub_connection> m_connection;
    Concurrency::concurrent_queue<std::pair<SERVER_MESSAGE_TYPE, signalr::value>> m_sendQueue{};
    std::thread m_workerThread;

    void workerLoop();
    void connect();
    void sendMessageToClient(std::pair<SERVER_MESSAGE_TYPE, signalr::value> message) const;
    void wait(int duration, std::function<bool()> predicate = []() { return false; });
};
