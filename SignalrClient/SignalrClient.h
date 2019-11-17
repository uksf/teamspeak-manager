#pragma once

#include "SignalrCommon.h"
#include "../Common/Lockable.h"
#include "../Common/IMessage.h"
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
	void initialize(std::function<void()> connectedCallback, std::function<void(IMessage*)> procedureCallback) override;
	void shutdown() override;
	void sendMessage(IMessage* message) override;

	DECLARE_MEMBER(BOOL, ClientConnecting)
	DECLARE_MEMBER(BOOL, ClientConnected)
	DECLARE_MEMBER(BOOL, ShuttingDown)

	BOOL getConnected() override {
		return this->getClientConnected();
	}

	void setConnected(const BOOL value) override {
		this->setClientConnected(value);
	}

private:
	std::function<void()> m_connectedCallback;
	std::function<void(IMessage*)> m_procedureCallback;
	std::shared_ptr<signalr::hub_connection> m_connection;
	Concurrency::concurrent_queue<IMessage*> m_sendQueue{};
	std::thread m_workerThread;

	void workerLoop();
	void connect();
	void sendMessageToClient(std::string message) const;
};
