#pragma once

#include "ManagerCommon.h"
#include "../Common/Singleton.h"
#include "../Common/Lockable.h"
#include <functional>
#include <concurrent_queue.h>

class Engine final : public Singleton<Engine>, public Lockable {
public:
	Engine() = default;
	~Engine() = default;

	void initialize();

	void start();
	void stop();

    [[nodiscard]] STATE getState() const;

	void addToFunctionQueue(std::function<void()> function);
	void addToSendQueue(SERVER_MESSAGE_TYPE type, signalr::value value);
	void addToReceiveQueue(ClientMessage message);

private:
	STATE m_state = STATE::STOPPED;
	std::thread m_workerThread;
	Concurrency::concurrent_queue<std::function<void()>> m_functionQueue{};
	Concurrency::concurrent_queue<std::pair<SERVER_MESSAGE_TYPE, signalr::value>> m_sendQueue{};
	Concurrency::concurrent_queue<ClientMessage> m_receiveQueue{};


	void setNewState(STATE state);
	void clearQueues();
	void doWork();
};
