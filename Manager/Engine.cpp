#include <future>
#include "Engine.h"
#include "../SignalrClient/SignalrClient.h"
#include "Data.h"
#include "ProcedureEngine.h"

extern TS3Functions ts3Functions;

void Engine::initialize() {
	logTSMessage("Engine: Initialising");
	this->clearQueues();
	Data::getInstance()->initialize();
	ProcedureEngine::getInstance()->initialize();
	SignalrClient::getInstance()->initialize();
	logTSMessage("Engine: Initialization complete");
}

void Engine::start() {
	logTSMessage("Engine: Starting");
	this->clearQueues();
	this->m_workerThread = std::thread(&Engine::doWork, this);
	this->setNewState(STATE::RUNNING);
	logTSMessage("Engine: Started");
}

void Engine::stop() {
	if (this->getState() == STATE::STOPPED || this->getState() == STATE::STOPPING) return;

	logTSMessage("Engine: Stopping");

    SignalrClient::getInstance()->requestDisconnect();
    while (SignalrClient::getInstance()->getState() != CONNECTION_STATE::DISCONNECTED) {
		Sleep(1);
    }

	this->setNewState(STATE::STOPPING);
	if (this->m_workerThread.joinable()) {
		this->m_workerThread.join();
	}

	this->clearQueues();
	Data::getInstance()->resetClientMaps();
	this->setNewState(STATE::STOPPED);
	logTSMessage("Engine: Stopped");
}

STATE Engine::getState() const {
	return this->m_state;
}

void Engine::setNewState(const STATE state) {
	{
		std::lock_guard<std::mutex> lock(m_lockable_mutex);
		this->m_state = state;
	}
}

void Engine::clearQueues() {
	{
		std::lock_guard<std::mutex> lock(m_lockable_mutex);
		this->m_functionQueue.clear();
		this->m_sendQueue.clear();
		this->m_receiveQueue.clear();
	}
}

void Engine::doWork() {
	while (this->getState() == STATE::RUNNING) {
		SignalrClient::getInstance()->updateConnectionState();
		const auto signalrState = SignalrClient::getInstance()->getState();

        if (SignalrClient::getInstance()->isDisconnectRequested() && signalrState != CONNECTION_STATE::CONNECTING) {
			logTSMessage("Engine: Running disconnect request");
			SignalrClient::getInstance()->disconnect();
        } else {
			// If disconnected
			if (signalrState == CONNECTION_STATE::DISCONNECTED) {
				// Connect
				logTSMessage("Engine: Not connected");
				SignalrClient::getInstance()->connect();
			}

			// Run queued functions (all)
			while (!this->m_functionQueue.empty()) {
				logTSMessage("Engine: Running function");
				std::function<void()> function;
				if (this->m_functionQueue.try_pop(function)) {
					function();
				}
			}

			// If connected
			if (signalrState == CONNECTION_STATE::CONNECTED) {
				// Send to signalr (x1)
				if (!this->m_sendQueue.empty()) {
					std::pair<SERVER_MESSAGE_TYPE, signalr::value> message;
					if (this->m_sendQueue.try_pop(message)) {
						SignalrClient::getInstance()->sendMessage(message);
					}
				}

				// Run received from signalr (x1)
				if (!this->m_receiveQueue.empty()) {
					ClientMessage message;
					if (this->m_receiveQueue.try_pop(message)) {
						ProcedureEngine::getInstance()->runProcedure(message);
					}
				}
			}
        }

		// Sleep 1ms
		Sleep(1);
	}
}

void Engine::addToFunctionQueue(const std::function<void()> function) {
	this->m_functionQueue.push(function);
}

void Engine::addToSendQueue(SERVER_MESSAGE_TYPE type, signalr::value value) {
	std::lock_guard<std::mutex> lock(m_lockable_mutex);
	this->m_sendQueue.push(std::pair<SERVER_MESSAGE_TYPE, signalr::value>(type, value));
}

void Engine::addToReceiveQueue(const ClientMessage message) {
	this->m_receiveQueue.push(message);
}
